#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // Set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // Set M Exception Program Counter to main, for mret.
  // Requires gcc -mcmodel=medany
  w_mepc((uint64)main);

  // Disable paging for now.
  w_satp(0);

  // Delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE);

  // Configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  w_pmpaddr0(0x3fffffffffffffull);
  w_pmpcfg0(0xf);

  // Ask for clock interrupts.
  timerinit();

  // Keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);

  // Switch to supervisor mode and jump to main().
  asm volatile("mret");
}

// Ask each hart to generate timer interrupts.
void
timerinit()
{
  // Enable supervisor-mode timer interrupts.
  w_mie(r_mie() | MIE_STIE);
  
  // Enable the sstc extension (i.e. stimecmp).
  w_menvcfg(r_menvcfg() | (1L << 63)); 
  
  // Allow supervisor to use stimecmp and time.
  w_mcounteren(r_mcounteren() | 2);
  
  // Ask for the very first timer interrupt.
  w_stimecmp(r_time() + 1000000);
}