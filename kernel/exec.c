#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"

// Debug and security configuration
#define DEBUG_EXEC 1
#define DEBUG_ELF 1
#define ENABLE_SECURITY_CHECKS 1

// Security limits
#define MAX_EXEC_SIZE (256 * 1024 * 1024)  // 256MB executable limit
#define MAX_ARGS 64                         // Maximum number of arguments
#define MAX_ARG_LEN 4096                   // Maximum length of single argument

// Function prototypes
static int loadseg(pagetable_t, uint64, struct inode *, uint, uint);
static int validate_elf_header(struct elfhdr *elf);
static int validate_program_header(struct proghdr *ph, int i);
static void log_exec_attempt(char *path, int pid, int success);

// Global execution statistics
static struct {
    uint64 total_execs;
    uint64 failed_execs;
    uint64 security_violations;
    struct spinlock lock;
} exec_stats = {0, 0, 0, {}};

// Initialize exec statistics
void exec_init(void) {
    initlock(&exec_stats.lock, "exec_stats");
}

// Map ELF permissions to PTE permission bits with enhanced validation
int flags2perm(int flags) {
    int perm = 0;
    
    // Validate flags range
    if (flags < 0 || flags > 7) {
        if (DEBUG_ELF) {
            printf("[ERROR] flags2perm: invalid flags 0x%x\n", flags);
        }
        return 0;  // No permissions for invalid flags
    }
    
    if (flags & ELF_PROG_FLAG_EXEC)  // Execute
        perm = PTE_X;
    if (flags & ELF_PROG_FLAG_WRITE)  // Write
        perm |= PTE_W;
    if (flags & ELF_PROG_FLAG_READ)  // Read
        perm |= PTE_R;
    
    // Ensure readable pages are also accessible in user mode
    if (perm & (PTE_R | PTE_W | PTE_X))
        perm |= PTE_U;
    
    if (DEBUG_ELF) {
        printf("[DEBUG] flags2perm: flags=0x%x -> perm=0x%x\n", flags, perm);
    }
    
    return perm;
}

// Enhanced ELF header validation
static int validate_elf_header(struct elfhdr *elf) {
    if (!elf) {
        printf("[ERROR] validate_elf_header: null pointer\n");
        return -1;
    }
    
    // Check magic number
    if (elf->magic != ELF_MAGIC) {
        if (DEBUG_ELF) {
            printf("[ERROR] Invalid ELF magic: 0x%x (expected 0x%x)\n", 
                   elf->magic, ELF_MAGIC);
        }
        return -1;
    }
    
    // Validate program header count
    if (elf->phnum == 0 || elf->phnum > 16) {  // Reasonable limits
        if (DEBUG_ELF) {
            printf("[ERROR] Invalid program header count: %d\n", elf->phnum);
        }
        return -1;
    }
    
    // Validate entry point
    if (elf->entry == 0 || elf->entry >= MAXVA) {
        if (DEBUG_ELF) {
            printf("[ERROR] Invalid entry point: 0x%p\n", elf->entry);
        }
        return -1;
    }
    
    // Additional security checks
    if (ENABLE_SECURITY_CHECKS) {
        // Check for reasonable program header offset
        if (elf->phoff < sizeof(struct elfhdr) || elf->phoff > MAX_EXEC_SIZE) {
            printf("[SECURITY] Suspicious program header offset: 0x%p\n", elf->phoff);
            return -1;
        }
        
        // Validate header size
        if (elf->ehsize != sizeof(struct elfhdr)) {
            printf("[SECURITY] Invalid ELF header size: %d\n", elf->ehsize);
            return -1;
        }
    }
    
    return 0;
}

// Enhanced program header validation
static int validate_program_header(struct proghdr *ph, int i) {
    if (!ph) {
        printf("[ERROR] validate_program_header: null pointer\n");
        return -1;
    }
    
    // Only process LOAD segments
    if (ph->type != ELF_PROG_LOAD) {
        return 0;  // Skip, but not an error
    }
    
    // Validate sizes
    if (ph->memsz < ph->filesz) {
        if (DEBUG_ELF) {
            printf("[ERROR] Segment %d: memsz(0x%x) < filesz(0x%x)\n", 
                   i, ph->memsz, ph->filesz);
        }
        return -1;
    }
    
    // Check for integer overflow
    if (ph->vaddr + ph->memsz < ph->vaddr) {
        if (DEBUG_ELF) {
            printf("[ERROR] Segment %d: address overflow\n", i);
        }
        return -1;
    }
    
    // Validate alignment
    if (ph->vaddr % PGSIZE != 0) {
        if (DEBUG_ELF) {
            printf("[ERROR] Segment %d: not page aligned (0x%p)\n", i, ph->vaddr);
        }
        return -1;
    }
    
    // Validate virtual address range
    if (ph->vaddr >= MAXVA || ph->vaddr + ph->memsz >= MAXVA) {
        if (DEBUG_ELF) {
            printf("[ERROR] Segment %d: virtual address out of range\n", i);
        }
        return -1;
    }
    
    // Security check: prevent executable stack (W^X violation)
    if (ENABLE_SECURITY_CHECKS) {
        if ((ph->flags & (ELF_PROG_FLAG_WRITE | ELF_PROG_FLAG_EXEC)) == 
            (ELF_PROG_FLAG_WRITE | ELF_PROG_FLAG_EXEC)) {
            printf("[SECURITY] Segment %d: writable and executable (W^X violation)\n", i);
            acquire(&exec_stats.lock);
            exec_stats.security_violations++;
            release(&exec_stats.lock);
            return -1;
        }
        
        // Check for suspicious file offsets
        if (ph->off > MAX_EXEC_SIZE) {
            printf("[SECURITY] Segment %d: suspicious file offset 0x%p\n", i, ph->off);
            return -1;
        }
    }
    
    return 1;  // Valid LOAD segment
}

// Log execution attempts for security monitoring
static void log_exec_attempt(char *path, int pid, int success) {
    if (DEBUG_EXEC) {
        printf("[EXEC_LOG] pid=%d path=%s result=%s\n", 
               pid, path ? path : "(null)", success ? "SUCCESS" : "FAILED");
    }
}

// Enhanced exec() system call implementation
int kexec(char *path, char **argv) {
    char *s, *last;
    int i, off;
    uint64 argc, sz = 0, sp, ustack[MAXARG], stackbase;
    struct elfhdr elf;
    struct inode *ip;
    struct proghdr ph;
    pagetable_t pagetable = 0, oldpagetable;
    struct proc *p = myproc();
    int exec_success = 0;

    // Update statistics
    acquire(&exec_stats.lock);
    exec_stats.total_execs++;
    release(&exec_stats.lock);

    if (DEBUG_EXEC) {
        printf("[DEBUG] kexec: path=%s pid=%d\n", path ? path : "(null)", p->pid);
    }

    // Enhanced argument validation
    if (!path) {
        printf("[ERROR] kexec: null path\n");
        goto bad;
    }

    if (!argv) {
        printf("[ERROR] kexec: null argv\n");
        goto bad;
    }

    // Validate path length
    int path_len = strlen(path);
    if (path_len == 0 || path_len >= MAXPATH) {
        printf("[ERROR] kexec: invalid path length %d\n", path_len);
        goto bad;
    }

    // Security check: validate path characters
    if (ENABLE_SECURITY_CHECKS) {
        for (i = 0; i < path_len; i++) {
            if (path[i] < 0x20 || path[i] > 0x7E) {
                printf("[SECURITY] kexec: invalid character in path\n");
                goto bad;
            }
        }
    }

    begin_op();

    // Open the executable file with enhanced error handling
    if ((ip = namei(path)) == 0) {
        end_op();
        printf("[ERROR] kexec: cannot find %s\n", path);
        goto bad;
    }
    ilock(ip);

    // Enhanced file validation
    if (ip->type != T_FILE) {
        printf("[ERROR] kexec: %s is not a regular file (type=%d)\n", 
               path, ip->type);
        goto bad;
    }

    // Check file size limits
    if (ENABLE_SECURITY_CHECKS && ip->size > MAX_EXEC_SIZE) {
        printf("[SECURITY] kexec: file too large (%d bytes)\n", ip->size);
        goto bad;
    }

    if (ip->size < sizeof(struct elfhdr)) {
        printf("[ERROR] kexec: file too small for ELF header\n");
        goto bad;
    }

    // Read the ELF header with validation
    if (readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) {
        printf("[ERROR] kexec: cannot read ELF header\n");
        goto bad;
    }

    // Validate ELF header
    if (validate_elf_header(&elf) < 0) {
        goto bad;
    }

    if (DEBUG_ELF) {
        printf("[DEBUG] kexec: ELF header valid, entry=0x%p phnum=%d\n", 
               elf.entry, elf.phnum);
    }

    // Create new page table
    if ((pagetable = proc_pagetable(p)) == 0) {
        printf("[ERROR] kexec: cannot create page table\n");
        goto bad;
    }

    // Load program segments into memory with enhanced validation
    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
        if (readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) {
            printf("[ERROR] kexec: cannot read program header %d\n", i);
            goto bad;
        }
        
        int validation_result = validate_program_header(&ph, i);
        if (validation_result < 0) {
            goto bad;
        }
        if (validation_result == 0) {
            continue;  // Skip non-LOAD segments
        }

        if (DEBUG_ELF) {
            printf("[DEBUG] kexec: loading segment %d vaddr=0x%p memsz=0x%x filesz=0x%x flags=0x%x\n",
                   i, ph.vaddr, ph.memsz, ph.filesz, ph.flags);
        }

        uint64 sz1;
        if ((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, 
                           flags2perm(ph.flags))) == 0) {
            printf("[ERROR] kexec: cannot allocate memory for segment %d\n", i);
            goto bad;
        }
        sz = sz1;
        
        // Load segment data from file if present
        if (ph.filesz > 0) {
            if (loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0) {
                printf("[ERROR] kexec: cannot load segment %d\n", i);
                goto bad;
            }
        }
    }
    
    iunlockput(ip);
    end_op();
    ip = 0;

    p = myproc();
    uint64 oldsz = p->sz;

    // Allocate user stack with enhanced guard page setup
    sz = PGROUNDUP(sz);
    uint64 sz1;
    if ((sz1 = uvmalloc(pagetable, sz, sz + (USERSTACK + 1) * PGSIZE, PTE_W)) == 0) {
        printf("[ERROR] kexec: cannot allocate stack\n");
        goto bad;
    }
    sz = sz1;
    
    // Create stack guard page (no permissions) for stack overflow protection
    uvmclear(pagetable, sz - (USERSTACK + 1) * PGSIZE);
    sp = sz;
    stackbase = sp - USERSTACK * PGSIZE;

    if (DEBUG_EXEC) {
        printf("[DEBUG] kexec: stack allocated at 0x%p-0x%p (guard at 0x%p)\n", 
               stackbase, sp, sz - (USERSTACK + 1) * PGSIZE);
    }

    // Enhanced argument processing and validation
    for (argc = 0; argv[argc]; argc++) {
        if (argc >= MAXARG) {
            printf("[ERROR] kexec: too many arguments (%d)\n", argc);
            goto bad;
        }
        
        // Validate each argument
        if (!argv[argc]) {
            printf("[ERROR] kexec: null argument %d\n", argc);
            goto bad;
        }
        
        int arg_len = strlen(argv[argc]);
        if (ENABLE_SECURITY_CHECKS && arg_len > MAX_ARG_LEN) {
            printf("[SECURITY] kexec: argument %d too long (%d chars)\n", argc, arg_len);
            goto bad;
        }
    }

    if (DEBUG_EXEC) {
        printf("[DEBUG] kexec: %d arguments validated\n", argc);
    }

    // Copy argument strings onto stack with overflow protection
    for (int j = 0; j < argc; j++) {
        int len = strlen(argv[j]);
        sp -= len + 1;
        sp -= sp % 16; // RISC-V sp must be 16-byte aligned
        
        if (sp < stackbase) {
            printf("[ERROR] kexec: stack overflow copying arguments\n");
            goto bad;
        }
        
        if (copyout(pagetable, sp, argv[j], len + 1) < 0) {
            printf("[ERROR] kexec: cannot copy argument %d\n", j);
            goto bad;
        }
        
        ustack[j] = sp;
        
        if (DEBUG_EXEC) {
            printf("[DEBUG] kexec: arg[%d] = \"%s\" at 0x%p\n", j, argv[j], sp);
        }
    }
    ustack[argc] = 0;

    // Push argv array onto stack
    sp -= (argc + 1) * sizeof(uint64);
    sp -= sp % 16;
    
    if (sp < stackbase) {
        printf("[ERROR] kexec: stack overflow copying argv array\n");
        goto bad;
    }
    
    if (copyout(pagetable, sp, (char *)ustack, (argc + 1) * sizeof(uint64)) < 0) {
        printf("[ERROR] kexec: cannot copy argv array\n");
        goto bad;
    }

    // Set up registers for user main(argc, argv)
    p->trapframe->a1 = sp;

    // Save program name for debugging
    for (last = s = path; *s; s++)
        if (*s == '/')
            last = s + 1;
    safestrcpy(p->name, last, sizeof(p->name));
    
    // Commit to the user image
    oldpagetable = p->pagetable;
    p->pagetable = pagetable;
    p->sz = sz;
    p->trapframe->epc = elf.entry;  // Initial program counter
    p->trapframe->sp = sp;          // Initial stack pointer
    
    // Free old page table
    proc_freepagetable(oldpagetable, oldsz);

    exec_success = 1;
    log_exec_attempt(path, p->pid, 1);

    if (DEBUG_EXEC) {
        printf("[DEBUG] kexec: success pid=%d entry=0x%p sp=0x%p argc=%d\n",
               p->pid, elf.entry, sp, argc);
    }

    return argc; // This ends up in a0, the first argument to main(argc, argv)

bad:
    if (pagetable)
        proc_freepagetable(pagetable, sz);
    if (ip) {
        iunlockput(ip);
        end_op();
    }
    
    acquire(&exec_stats.lock);
    exec_stats.failed_execs++;
    release(&exec_stats.lock);
    
    log_exec_attempt(path, p ? p->pid : -1, 0);
    
    if (DEBUG_EXEC) {
        printf("[DEBUG] kexec: failed for path=%s\n", path ? path : "(null)");
    }
    
    return -1;
}

// Enhanced segment loader with security checks
static int loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint sz) {
    uint i, n;
    uint64 pa;

    if (DEBUG_ELF) {
        printf("[DEBUG] loadseg: va=0x%p offset=%d sz=%d\n", va, offset, sz);
    }

    // Enhanced parameter validation
    if (!pagetable) {
        printf("[ERROR] loadseg: null pagetable\n");
        return -1;
    }
    
    if (!ip) {
        printf("[ERROR] loadseg: null inode\n");
        return -1;
    }

    if (va % PGSIZE != 0) {
        printf("[ERROR] loadseg: va 0x%p not page aligned\n", va);
        return -1;
    }
    
    if (sz == 0) {
        if (DEBUG_ELF) {
            printf("[DEBUG] loadseg: zero size, nothing to load\n");
        }
        return 0;
    }

    // Validate offset and size against file size
    if (ENABLE_SECURITY_CHECKS) {
        if (offset >= ip->size || offset + sz > ip->size) {
            printf("[SECURITY] loadseg: read beyond file bounds (offset=%d, sz=%d, file_size=%d)\n",
                   offset, sz, ip->size);
            return -1;
        }
    }

    // Load data page by page with validation
    for (i = 0; i < sz; i += PGSIZE) {
        pa = walkaddr(pagetable, va + i);
        if (pa == 0) {
            printf("[ERROR] loadseg: address 0x%p should exist but is unmapped\n", va + i);
            return -1;
        }
        
        if (sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;
            
        if (readi(ip, 0, (uint64)pa, offset + i, n) != n) {
            printf("[ERROR] loadseg: read failed at offset %d (requested %d bytes)\n", 
                   offset + i, n);
            return -1;
        }
    }
    
    if (DEBUG_ELF) {
        printf("[DEBUG] loadseg: loaded %d bytes successfully\n", sz);
    }
    
    return 0;
}

// Print execution statistics
void print_exec_stats(void) {
    acquire(&exec_stats.lock);
    printf("=== Exec Statistics ===\n");
    printf("Total executions: %d\n", exec_stats.total_execs);
    printf("Failed executions: %d\n", exec_stats.failed_execs);
    printf("Security violations: %d\n", exec_stats.security_violations);
    if (exec_stats.total_execs > 0) {
        printf("Success rate: %d%%\n", 
               (exec_stats.total_execs - exec_stats.failed_execs) * 100 / exec_stats.total_execs);
    }
    release(&exec_stats.lock);
}