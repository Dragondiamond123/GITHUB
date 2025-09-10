#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

// Debug configuration
#define DEBUG_PROCESS_SYSCALLS 1
#define DEBUG_MEMORY_MGMT 1
#define DEBUG_PROCESS_LIFECYCLE 1
#define DEBUG_SCHEDULING 1
#define ENABLE_SECURITY_CHECKS 1

// Process syscall statistics
static struct {
  uint64 exit_count;
  uint64 getpid_count;
  uint64 fork_count;
  uint64 wait_count;
  uint64 sbrk_count;
  uint64 pause_count;
  uint64 kill_count;
  uint64 uptime_count;
  uint64 failed_operations;
  struct spinlock lock;
} proc_stats;

// Initialize process syscall statistics
void
proc_stats_init(void)
{
  initlock(&proc_stats.lock, "proc_stats");
  proc_stats.exit_count = 0;
  proc_stats.getpid_count = 0;
  proc_stats.fork_count = 0;
  proc_stats.wait_count = 0;
  proc_stats.sbrk_count = 0;
  proc_stats.pause_count = 0;
  proc_stats.kill_count = 0;
  proc_stats.uptime_count = 0;
  proc_stats.failed_operations = 0;
}

// Print process syscall statistics
void
print_proc_stats(void)
{
  acquire(&proc_stats.lock);
  printf("=== Process Syscall Statistics ===\n");
  printf("exit: %d, getpid: %d, fork: %d\n", 
         proc_stats.exit_count, proc_stats.getpid_count, proc_stats.fork_count);
  printf("wait: %d, sbrk: %d, pause: %d\n", 
         proc_stats.wait_count, proc_stats.sbrk_count, proc_stats.pause_count);
  printf("kill: %d, uptime: %d\n", 
         proc_stats.kill_count, proc_stats.uptime_count);
  printf("Failed operations: %d\n", proc_stats.failed_operations);
  release(&proc_stats.lock);
}

uint64
sys_exit(void)
{
  int n;
  struct proc *p = myproc();
  
  acquire(&proc_stats.lock);
  proc_stats.exit_count++;
  release(&proc_stats.lock);
  
  argint(0, &n);
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    printf("[SYSCALL] exit: pid=%d status=%d name=%s\n", p->pid, n, p->name);
  }
  
  // Validate exit status
  if(ENABLE_SECURITY_CHECKS) {
    if(n < -128 || n > 127) {
      printf("[WARNING] exit: unusual exit status %d from pid=%d\n", n, p->pid);
    }
  }
  
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  struct proc *p = myproc();
  
  acquire(&proc_stats.lock);
  proc_stats.getpid_count++;
  release(&proc_stats.lock);
  
  if(DEBUG_PROCESS_SYSCALLS) {
    printf("[SYSCALL] getpid: returning %d\n", p->pid);
  }
  
  return p->pid;
}

uint64
sys_fork(void)
{
  struct proc *p = myproc();
  int child_pid;
  
  acquire(&proc_stats.lock);
  proc_stats.fork_count++;
  release(&proc_stats.lock);
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    printf("[SYSCALL] fork: parent pid=%d name=%s\n", p->pid, p->name);
  }
  
  // Security check: reasonable fork limits
  if(ENABLE_SECURITY_CHECKS) {
    static int recent_forks = 0;
    static uint64 last_fork_time = 0;
    uint64 current_time = r_time();
    
    if(current_time - last_fork_time < 1000000) {  // Less than ~0.1 second
      recent_forks++;
      if(recent_forks > 10) {
        printf("[SECURITY] fork bomb detection: pid=%d excessive forks\n", p->pid);
        acquire(&proc_stats.lock);
        proc_stats.failed_operations++;
        release(&proc_stats.lock);
        return -1;
      }
    } else {
      recent_forks = 0;
    }
    last_fork_time = current_time;
  }
  
  child_pid = kfork();
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    if(child_pid > 0) {
      printf("[SYSCALL] fork: parent pid=%d created child pid=%d\n", p->pid, child_pid);
    } else if(child_pid == 0) {
      printf("[SYSCALL] fork: child process pid=%d started\n", myproc()->pid);
    } else {
      printf("[ERROR] fork: failed for parent pid=%d\n", p->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
    }
  }
  
  return child_pid;
}

uint64
sys_wait(void)
{
  uint64 p;
  struct proc *proc = myproc();
  
  acquire(&proc_stats.lock);
  proc_stats.wait_count++;
  release(&proc_stats.lock);
  
  argaddr(0, &p);
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    printf("[SYSCALL] wait: pid=%d waiting for child\n", proc->pid);
  }
  
  // Validate user pointer
  if(ENABLE_SECURITY_CHECKS && p != 0) {
    if(p >= proc->sz || p + sizeof(int) > proc->sz) {
      printf("[SECURITY] wait: invalid status pointer 0x%p from pid=%d\n", p, proc->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
  }
  
  int result = kwait(p);
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    if(result > 0) {
      printf("[SYSCALL] wait: pid=%d reaped child pid=%d\n", proc->pid, result);
    } else if(result == -1) {
      printf("[SYSCALL] wait: pid=%d no children to wait for\n", proc->pid);
    }
  }
  
  return result;
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;
  struct proc *p = myproc();

  acquire(&proc_stats.lock);
  proc_stats.sbrk_count++;
  release(&proc_stats.lock);

  argint(0, &n);
  argint(1, &t);
  addr = p->sz;

  if(DEBUG_MEMORY_MGMT) {
    printf("[SYSCALL] sbrk: pid=%d current_sz=0x%p increment=%d type=%s\n", 
           p->pid, addr, n, (t == SBRK_EAGER) ? "eager" : "lazy");
  }

  // Security checks
  if(ENABLE_SECURITY_CHECKS) {
    // Prevent excessive memory allocation
    if(n > 100*1024*1024) {  // 100MB limit per sbrk call
      printf("[SECURITY] sbrk: excessive allocation request %d bytes from pid=%d\n", 
             n, p->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
    
    // Check for total process size limits
    if(addr + n > 512*1024*1024) {  // 512MB total process limit
      printf("[SECURITY] sbrk: process size limit exceeded for pid=%d\n", p->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
    
    // Check for integer overflow
    if(n > 0 && addr + n < addr) {
      printf("[SECURITY] sbrk: integer overflow detected pid=%d\n", p->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
  }

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      if(DEBUG_MEMORY_MGMT) {
        printf("[ERROR] sbrk: growproc failed for pid=%d\n", p->pid);
      }
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the process uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr) {
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
    p->sz += n;
    
    if(DEBUG_MEMORY_MGMT) {
      printf("[SYSCALL] sbrk: lazy allocation, new size=0x%p\n", p->sz);
    }
  }
  
  if(DEBUG_MEMORY_MGMT) {
    printf("[SYSCALL] sbrk: success, returning addr=0x%p new_sz=0x%p\n", 
           addr, p->sz);
  }
  
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;
  struct proc *p = myproc();

  acquire(&proc_stats.lock);
  proc_stats.pause_count++;
  release(&proc_stats.lock);

  argint(0, &n);
  
  if(DEBUG_SCHEDULING) {
    printf("[SYSCALL] pause: pid=%d sleep_ticks=%d\n", p->pid, n);
  }
  
  if(n < 0) {
    n = 0;
    if(DEBUG_SCHEDULING) {
      printf("[WARNING] pause: negative sleep time corrected to 0\n");
    }
  }
  
  // Security check: prevent excessive sleep times
  if(ENABLE_SECURITY_CHECKS && n > 100000) {  // ~10 seconds max
    printf("[SECURITY] pause: excessive sleep time %d from pid=%d\n", n, p->pid);
    n = 100000;
  }
  
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(p)){
      release(&tickslock);
      if(DEBUG_SCHEDULING) {
        printf("[SYSCALL] pause: pid=%d killed during sleep\n", p->pid);
      }
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  
  if(DEBUG_SCHEDULING) {
    printf("[SYSCALL] pause: pid=%d woke up after %d ticks\n", 
           p->pid, ticks - ticks0);
  }
  
  return 0;
}

uint64
sys_kill(void)
{
  int pid;
  struct proc *p = myproc();

  acquire(&proc_stats.lock);
  proc_stats.kill_count++;
  release(&proc_stats.lock);

  argint(0, &pid);
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    printf("[SYSCALL] kill: pid=%d attempting to kill pid=%d\n", p->pid, pid);
  }
  
  // Security checks
  if(ENABLE_SECURITY_CHECKS) {
    // Prevent killing init process (pid 1)
    if(pid == 1) {
      printf("[SECURITY] kill: attempt to kill init process from pid=%d\n", p->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
    
    // Validate PID range
    if(pid <= 0 || pid > 32768) {
      printf("[SECURITY] kill: invalid pid %d from pid=%d\n", pid, p->pid);
      acquire(&proc_stats.lock);
      proc_stats.failed_operations++;
      release(&proc_stats.lock);
      return -1;
    }
    
    // Prevent self-kill in some contexts
    if(pid == p->pid) {
      printf("[WARNING] kill: process %d killing itself\n", p->pid);
    }
  }
  
  int result = kkill(pid);
  
  if(DEBUG_PROCESS_LIFECYCLE) {
    if(result == 0) {
      printf("[SYSCALL] kill: pid=%d successfully marked pid=%d for killing\n", 
             p->pid, pid);
    } else {
      printf("[ERROR] kill: pid=%d failed to kill pid=%d (not found)\n", 
             p->pid, pid);
    }
  }
  
  return result;
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&proc_stats.lock);
  proc_stats.uptime_count++;
  release(&proc_stats.lock);

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  
  if(DEBUG_PROCESS_SYSCALLS) {
    printf("[SYSCALL] uptime: returning %d ticks\n", xticks);
  }
  
  return xticks;
}

// Debug function to dump current process information
void
debug_current_proc(void)
{
  struct proc *p = myproc();
  if(!p) {
    printf("No current process\n");
    return;
  }
  
  printf("=== Current Process Debug ===\n");
  printf("PID: %d\n", p->pid);
  printf("Name: %s\n", p->name);
  printf("State: %d\n", p->state);
  printf("Size: 0x%p (%d bytes)\n", p->sz, p->sz);
  printf("Stack: 0x%p\n", p->kstack);
  printf("Page table: 0x%p\n", p->pagetable);
  printf("Parent: 0x%p\n", p->parent);
  printf("Killed: %d\n", killed(p));
  printf("===========================\n");
}

// Memory usage analysis
void
analyze_memory_usage(void)
{
  struct proc *p = myproc();
  if(!p) return;
  
  printf("=== Memory Analysis for PID %d ===\n", p->pid);
  printf("Process size: %d KB\n", p->sz / 1024);
  printf("Stack usage: ~4 KB (kernel stack)\n");
  
  // Estimate heap usage (simplified)
  if(p->sz > PGSIZE) {
    printf("Estimated heap: %d KB\n", (p->sz - PGSIZE) / 1024);
  } else {
    printf("Estimated heap: 0 KB\n");
  }
  printf("==============================\n");
}

// Security audit function
void
security_audit_process(void)
{
  struct proc *p = myproc();
  if(!p) return;
  
  printf("=== Security Audit PID %d ===\n", p->pid);
  printf("Process name: %s\n", p->name);
  printf("Memory size: %d bytes\n", p->sz);
  
  // Check for suspicious patterns
  if(p->sz > 50*1024*1024) {
    printf("[ALERT] Large memory usage detected\n");
  }
  
  // Check open file count
  int open_files = 0;
  for(int i = 0; i < NOFILE; i++) {
    if(p->ofile[i]) open_files++;
  }
  printf("Open files: %d/%d\n", open_files, NOFILE);
  
  if(open_files > NOFILE - 5) {
    printf("[ALERT] High file descriptor usage\n");
  }
  
  printf("==========================\n");
}