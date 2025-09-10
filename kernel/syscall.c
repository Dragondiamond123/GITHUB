#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Debug flags
#define DEBUG_SYSCALLS 1
#define DEBUG_ARGS 1
#define DEBUG_FETCHADDR 1
#define DEBUG_FETCHSTR 1

// Performance counters
static struct {
  uint64 syscall_count[22];  // Track calls per syscall
  uint64 total_calls;
  uint64 failed_calls;
  struct spinlock lock;
} syscall_stats;

static char *syscall_names[] = {
[SYS_fork]    "fork",
[SYS_exit]    "exit",
[SYS_wait]    "wait",
[SYS_pipe]    "pipe",
[SYS_read]    "read",
[SYS_kill]    "kill",
[SYS_exec]    "exec",
[SYS_fstat]   "fstat",
[SYS_chdir]   "chdir",
[SYS_dup]     "dup",
[SYS_getpid]  "getpid",
[SYS_sbrk]    "sbrk",
[SYS_pause]   "pause",
[SYS_uptime]  "uptime",
[SYS_open]    "open",
[SYS_write]   "write",
[SYS_mknod]   "mknod",
[SYS_unlink]  "unlink",
[SYS_link]    "link",
[SYS_mkdir]   "mkdir",
[SYS_close]   "close",
};

void
syscall_stats_init(void)
{
  initlock(&syscall_stats.lock, "syscall_stats");
  for(int i = 0; i < 22; i++)
    syscall_stats.syscall_count[i] = 0;
  syscall_stats.total_calls = 0;
  syscall_stats.failed_calls = 0;
}

void
print_syscall_stats(void)
{
  acquire(&syscall_stats.lock);
  printf("=== Syscall Statistics ===\n");
  printf("Total calls: %d\n", syscall_stats.total_calls);
  printf("Failed calls: %d\n", syscall_stats.failed_calls);
  printf("Success rate: %d%%\n", 
    syscall_stats.total_calls ? 
    (100 * (syscall_stats.total_calls - syscall_stats.failed_calls)) / syscall_stats.total_calls : 0);
  
  for(int i = 1; i < 22; i++) {
    if(syscall_stats.syscall_count[i] > 0) {
      printf("  %s: %d calls\n", syscall_names[i], syscall_stats.syscall_count[i]);
    }
  }
  release(&syscall_stats.lock);
}

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  
  if(DEBUG_FETCHADDR) {
    printf("[DEBUG] fetchaddr: pid=%d addr=0x%p sz=0x%p\n", p->pid, addr, p->sz);
  }
  
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) { // both tests needed, in case of overflow
    if(DEBUG_FETCHADDR) {
      printf("[ERROR] fetchaddr: address out of bounds pid=%d addr=0x%p sz=0x%p\n", 
             p->pid, addr, p->sz);
    }
    return -1;
  }
  
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0) {
    if(DEBUG_FETCHADDR) {
      printf("[ERROR] fetchaddr: copyin failed pid=%d addr=0x%p\n", p->pid, addr);
    }
    return -1;
  }
  
  if(DEBUG_FETCHADDR) {
    printf("[DEBUG] fetchaddr: success pid=%d addr=0x%p value=0x%p\n", p->pid, addr, *ip);
  }
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int len;
  
  if(DEBUG_FETCHSTR) {
    printf("[DEBUG] fetchstr: pid=%d addr=0x%p max=%d\n", p->pid, addr, max);
  }
  
  if((len = copyinstr(p->pagetable, buf, addr, max)) < 0) {
    if(DEBUG_FETCHSTR) {
      printf("[ERROR] fetchstr: copyinstr failed pid=%d addr=0x%p max=%d\n", 
             p->pid, addr, max);
    }
    return -1;
  }
  
  len = strlen(buf);
  if(DEBUG_FETCHSTR) {
    printf("[DEBUG] fetchstr: success pid=%d addr=0x%p str=\"%s\" len=%d\n", 
           p->pid, addr, buf, len);
  }
  
  return len;
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  uint64 val;
  
  switch (n) {
  case 0:
    val = p->trapframe->a0;
    break;
  case 1:
    val = p->trapframe->a1;
    break;
  case 2:
    val = p->trapframe->a2;
    break;
  case 3:
    val = p->trapframe->a3;
    break;
  case 4:
    val = p->trapframe->a4;
    break;
  case 5:
    val = p->trapframe->a5;
    break;
  default:
    printf("[ERROR] argraw: invalid argument number %d\n", n);
    panic("argraw");
    return -1;
  }
  
  if(DEBUG_ARGS) {
    printf("[DEBUG] argraw: arg%d = 0x%p (%d)\n", n, val, val);
  }
  
  return val;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
  if(DEBUG_ARGS) {
    printf("[DEBUG] argint: arg%d = %d\n", n, *ip);
  }
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  if(DEBUG_ARGS) {
    printf("[DEBUG] argaddr: arg%d = 0x%p\n", n, *ip);
  }
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  int len = fetchstr(addr, buf, max);
  
  if(DEBUG_ARGS) {
    if(len >= 0) {
      printf("[DEBUG] argstr: arg%d = \"%s\" (len=%d)\n", n, buf, len);
    } else {
      printf("[ERROR] argstr: arg%d failed\n", n);
    }
  }
  
  return len;
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_pause(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_pause]   sys_pause,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();
  uint64 start_time = r_time();
  uint64 ret_val;

  num = p->trapframe->a7;
  
  // Update statistics
  acquire(&syscall_stats.lock);
  syscall_stats.total_calls++;
  if(num > 0 && num < NELEM(syscalls)) {
    syscall_stats.syscall_count[num]++;
  }
  release(&syscall_stats.lock);
  
  if(DEBUG_SYSCALLS) {
    printf("[SYSCALL] pid=%d %s(%d) called\n", 
           p->pid, 
           (num > 0 && num < NELEM(syscall_names) && syscall_names[num]) ? 
           syscall_names[num] : "unknown", 
           num);
  }
  
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    ret_val = syscalls[num]();
    p->trapframe->a0 = ret_val;
    
    if(DEBUG_SYSCALLS) {
      uint64 end_time = r_time();
      printf("[SYSCALL] pid=%d %s returned %d (took %d cycles)\n", 
             p->pid, syscall_names[num], ret_val, end_time - start_time);
    }
  } else {
    printf("[ERROR] pid=%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
    ret_val = -1;
    
    acquire(&syscall_stats.lock);
    syscall_stats.failed_calls++;
    release(&syscall_stats.lock);
  }
  
  // Check for potential security issues
  if(num < 0 || num >= NELEM(syscalls)) {
    printf("[SECURITY] pid=%d attempted invalid syscall %d\n", p->pid, num);
  }
  
  // Validate return value for critical syscalls
  if(num == SYS_fork && ret_val < -1) {
    printf("[WARNING] fork returned invalid value %d\n", ret_val);
  } else if(num == SYS_open && ret_val < -1) {
    printf("[WARNING] open returned invalid fd %d\n", ret_val);
  }
}

// Debug function to dump current process state during syscall
void
syscall_debug_proc(void)
{
  struct proc *p = myproc();
  printf("=== Process Debug Info ===\n");
  printf("PID: %d\n", p->pid);
  printf("Name: %s\n", p->name);
  printf("State: %d\n", p->state);
  printf("Size: 0x%p\n", p->sz);
  printf("Trapframe: 0x%p\n", p->trapframe);
  if(p->trapframe) {
    printf("  PC: 0x%p\n", p->trapframe->epc);
    printf("  SP: 0x%p\n", p->trapframe->sp);
    printf("  Args: a0=0x%p a1=0x%p a2=0x%p\n", 
           p->trapframe->a0, p->trapframe->a1, p->trapframe->a2);
  }
  printf("========================\n");
}