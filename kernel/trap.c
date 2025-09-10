#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// Debug configuration
#define DEBUG_TRAPS 1
#define DEBUG_SYSCALLS 1
#define DEBUG_PAGEFAULTS 1
#define DEBUG_INTERRUPTS 1
#define DEBUG_CONTEXT_SWITCH 1

// Trap statistics
static struct {
  uint64 syscall_count;
  uint64 pagefault_count;
  uint64 interrupt_count;
  uint64 exception_count;
  uint64 unknown_trap_count;
  struct spinlock lock;
} trap_stats;

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

// Trap cause names for debugging
static char *scause_names[] = {
  [0] = "Instruction address misaligned",
  [1] = "Instruction access fault", 
  [2] = "Illegal instruction",
  [3] = "Breakpoint",
  [4] = "Load address misaligned",
  [5] = "Load access fault",
  [6] = "Store/AMO address misaligned", 
  [7] = "Store/AMO access fault",
  [8] = "Environment call from U-mode",
  [9] = "Environment call from S-mode",
  [12] = "Instruction page fault",
  [13] = "Load page fault",
  [15] = "Store/AMO page fault",
};

void
trapinit(void)
{
  initlock(&tickslock, "time");
  initlock(&trap_stats.lock, "trap_stats");
  
  // Initialize trap statistics
  trap_stats.syscall_count = 0;
  trap_stats.pagefault_count = 0;
  trap_stats.interrupt_count = 0;
  trap_stats.exception_count = 0;
  trap_stats.unknown_trap_count = 0;
  
  if(DEBUG_TRAPS) {
    printf("[TRAP] Trap system initialized\n");
  }
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
  if(DEBUG_TRAPS) {
    printf("[TRAP] Hart %d trap vector set to kernelvec (0x%p)\n", cpuid(), kernelvec);
  }
}

// Get trap cause name for debugging
static char*
get_scause_name(uint64 scause)
{
  if(scause & 0x8000000000000000L) {
    // Interrupt
    switch(scause & 0x7fffffffffffffffL) {
      case 1: return "Supervisor software interrupt";
      case 5: return "Supervisor timer interrupt";
      case 9: return "Supervisor external interrupt";
      default: return "Unknown interrupt";
    }
  } else {
    // Exception
    if(scause < sizeof(scause_names)/sizeof(scause_names[0]) && scause_names[scause]) {
      return scause_names[scause];
    }
    return "Unknown exception";
  }
}

// Debug function to dump trap context
void
debug_trap_context(uint64 scause, uint64 sepc, uint64 stval)
{
  struct proc *p = myproc();
  printf("=== TRAP DEBUG INFO ===\n");
  printf("PID: %d (%s)\n", p ? p->pid : -1, p ? p->name : "unknown");
  printf("Hart: %d\n", cpuid());
  printf("Cause: 0x%p (%s)\n", scause, get_scause_name(scause));
  printf("PC: 0x%p\n", sepc);
  printf("BadVAddr: 0x%p\n", stval);
  printf("Status: 0x%p\n", r_sstatus());
  if(p && p->trapframe) {
    printf("User SP: 0x%p\n", p->trapframe->sp);
    printf("Args: a0=0x%p a1=0x%p a2=0x%p a7=0x%p\n", 
           p->trapframe->a0, p->trapframe->a1, p->trapframe->a2, p->trapframe->a7);
  }
  printf("=====================\n");
}

//
// handle an interrupt, exception, or system call from user space.
// called from, and returns to, trampoline.S
// return value is user satp for trampoline.S to switch to.
//
uint64
usertrap(void)
{
  int which_dev = 0;
  uint64 scause, sepc, stval;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);  //DOC: kernelvec

  struct proc *p = myproc();
  
  // save user program counter.
  p->trapframe->epc = r_sepc();
  
  scause = r_scause();
  sepc = r_sepc();
  stval = r_stval();
  
  if(DEBUG_TRAPS) {
    printf("[TRAP] User trap: pid=%d cause=0x%p (%s) pc=0x%p\n", 
           p->pid, scause, get_scause_name(scause), sepc);
  }
  
  if(scause == 8){
    // system call
    acquire(&trap_stats.lock);
    trap_stats.syscall_count++;
    release(&trap_stats.lock);

    if(killed(p)) {
      if(DEBUG_SYSCALLS) {
        printf("[TRAP] Process %d killed before syscall\n", p->pid);
      }
      kexit(-1);
    }

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sepc, scause, and sstatus,
    // so enable only now that we're done with those registers.
    intr_on();

    if(DEBUG_SYSCALLS) {
      printf("[TRAP] Syscall %d from pid=%d\n", p->trapframe->a7, p->pid);
    }

    syscall();
  } else if((which_dev = devintr()) != 0){
    // Device interrupt
    acquire(&trap_stats.lock);
    trap_stats.interrupt_count++;
    release(&trap_stats.lock);
    
    if(DEBUG_INTERRUPTS) {
      printf("[TRAP] Device interrupt %d handled\n", which_dev);
    }
  } else if((scause == 15 || scause == 13) &&
            vmfault(p->pagetable, stval, (scause == 13)? 1 : 0) != 0) {
    // page fault on lazily-allocated page
    acquire(&trap_stats.lock);
    trap_stats.pagefault_count++;
    release(&trap_stats.lock);
    
    if(DEBUG_PAGEFAULTS) {
      printf("[TRAP] Page fault handled: pid=%d addr=0x%p %s\n", 
             p->pid, stval, (scause == 13) ? "load" : "store");
    }
  } else {
    // Unknown trap
    acquire(&trap_stats.lock);
    trap_stats.exception_count++;
    trap_stats.unknown_trap_count++;
    release(&trap_stats.lock);
    
    printf("[ERROR] usertrap(): unexpected scause 0x%p pid=%d\n", scause, p->pid);
    printf("            sepc=0x%p stval=0x%p\n", sepc, stval);
    debug_trap_context(scause, sepc, stval);
    setkilled(p);
  }

  if(killed(p)) {
    if(DEBUG_TRAPS) {
      printf("[TRAP] Process %d killed, exiting\n", p->pid);
    }
    kexit(-1);
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2) {
    if(DEBUG_CONTEXT_SWITCH) {
      printf("[TRAP] Timer interrupt, yielding CPU from pid=%d\n", p->pid);
    }
    yield();
  }

  prepare_return();

  // the user page table to switch to, for trampoline.S
  uint64 satp = MAKE_SATP(p->pagetable);
  
  if(DEBUG_TRAPS) {
    printf("[TRAP] Returning to user mode: pid=%d satp=0x%p\n", p->pid, satp);
  }

  // return to trampoline.S; satp value in a0.
  return satp;
}

//
// set up trapframe and control registers for a return to user space
//
void
prepare_return(void)
{
  struct proc *p = myproc();

  if(DEBUG_CONTEXT_SWITCH) {
    printf("[TRAP] Preparing return to user mode: pid=%d\n", p->pid);
  }

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(). because a trap from kernel
  // code to usertrap would be a disaster, turn off interrupts.
  intr_off();

  // send syscalls, interrupts, and exceptions to uservec in trampoline.S
  uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
  w_stvec(trampoline_uservec);

  // set up trapframe values that uservec will need when
  // the process next traps into the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // Validate trapframe setup
  if(p->trapframe->kernel_sp == 0) {
    panic("prepare_return: invalid kernel stack");
  }
  if(p->trapframe->kernel_satp == 0) {
    panic("prepare_return: invalid kernel page table");
  }

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);
  
  if(DEBUG_CONTEXT_SWITCH) {
    printf("[TRAP] Return setup complete: pc=0x%p sp=0x%p\n", 
           p->trapframe->epc, p->trapframe->sp);
  }
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  uint64 stval = r_stval();
  
  if(DEBUG_TRAPS) {
    printf("[KTRAP] Kernel trap: cause=0x%p (%s) pc=0x%p hart=%d\n", 
           scause, get_scause_name(scause), sepc, cpuid());
  }
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    // interrupt or trap from an unknown source
    printf("[ERROR] Unknown kernel trap:\n");
    printf("scause=0x%p (%s) sepc=0x%p stval=0x%p\n", 
           scause, get_scause_name(scause), sepc, stval);
    printf("sstatus=0x%p hart=%d\n", sstatus, cpuid());
    debug_trap_context(scause, sepc, stval);
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0) {
    if(DEBUG_CONTEXT_SWITCH) {
      printf("[KTRAP] Timer interrupt in kernel, yielding\n");
    }
    yield();
  }

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr()
{
  if(cpuid() == 0){
    acquire(&tickslock);
    ticks++;
    if(DEBUG_INTERRUPTS && (ticks % 100) == 0) {
      printf("[CLOCK] Tick %d\n", ticks);
    }
    wakeup(&ticks);
    release(&tickslock);
  }

  // ask for the next timer interrupt. this also clears
  // the interrupt request. 1000000 is about a tenth
  // of a second.
  w_stimecmp(r_time() + 1000000);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if(scause == 0x8000000000000009L){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(DEBUG_INTERRUPTS) {
      printf("[DEVINTR] External interrupt: irq=%d\n", irq);
    }

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("[WARNING] unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000005L){
    // timer interrupt.
    clockintr();
    return 2;
  } else {
    return 0;
  }
}

// Print trap statistics
void
print_trap_stats(void)
{
  acquire(&trap_stats.lock);
  printf("=== Trap Statistics ===\n");
  printf("Syscalls: %d\n", trap_stats.syscall_count);
  printf("Page faults: %d\n", trap_stats.pagefault_count);
  printf("Interrupts: %d\n", trap_stats.interrupt_count);
  printf("Exceptions: %d\n", trap_stats.exception_count);
  printf("Unknown traps: %d\n", trap_stats.unknown_trap_count);
  printf("Total: %d\n", trap_stats.syscall_count + trap_stats.pagefault_count + 
                        trap_stats.interrupt_count + trap_stats.exception_count);
  release(&trap_stats.lock);
}