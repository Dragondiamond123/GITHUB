//
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x

// Debug flags
#define DEBUG_CONSOLE 0
#define DEBUG_INPUT 0

//
// Send one character to the uart.
// Called by printf(), and to echo input characters,
// but not from write().
//
void
consputc(int c)
{
  if(c == BACKSPACE){
    // if the user typed backspace, overwrite with a space.
    uartputc_sync('\b'); 
    uartputc_sync(' '); 
    uartputc_sync('\b');
  } else {
    uartputc_sync(c);
  }
}

struct {
  struct spinlock lock;
  
  // input buffer
#define INPUT_BUF_SIZE 128
  char buf[INPUT_BUF_SIZE];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} cons;

//
// User write()s to the console go here.
//
int
consolewrite(int user_src, uint64 src, int n)
{
  char buf[32];
  int i = 0;

  if(DEBUG_CONSOLE) {
    printf("[DEBUG] consolewrite: user_src=%d src=0x%p n=%d\n", user_src, src, n);
  }

  if(n <= 0) {
    return 0;
  }

  while(i < n){
    int nn = sizeof(buf);
    if(nn > n - i)
      nn = n - i;
    if(either_copyin(buf, user_src, src + i, nn) == -1) {
      if(DEBUG_CONSOLE) {
        printf("[ERROR] consolewrite: copyin failed at offset %d\n", i);
      }
      break;
    }
    uartwrite(buf, nn);
    i += nn;
  }

  if(DEBUG_CONSOLE) {
    printf("[DEBUG] consolewrite: wrote %d bytes\n", i);
  }

  return i;
}

//
// User read()s from the console go here.
// Copy (up to) a whole input line to dst.
// user_dst indicates whether dst is a user or kernel address.
//
int
consoleread(int user_dst, uint64 dst, int n)
{
  uint target;
  int c;
  char cbuf;

  if(DEBUG_CONSOLE) {
    printf("[DEBUG] consoleread: user_dst=%d dst=0x%p n=%d\n", user_dst, dst, n);
  }

  if(n <= 0) {
    return 0;
  }

  target = n;
  acquire(&cons.lock);
  
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cons.buffer.
    while(cons.r == cons.w){
      if(killed(myproc())){
        release(&cons.lock);
        if(DEBUG_CONSOLE) {
          printf("[DEBUG] consoleread: process killed\n");
        }
        return -1;
      }
      sleep(&cons.r, &cons.lock);
    }

    c = cons.buf[cons.r++ % INPUT_BUF_SIZE];

    if(c == C('D')){  // end-of-file
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cons.r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.
    cbuf = c;
    if(either_copyout(user_dst, dst, &cbuf, 1) == -1) {
      if(DEBUG_CONSOLE) {
        printf("[ERROR] consoleread: copyout failed\n");
      }
      break;
    }

    dst++;
    --n;

    if(c == '\n'){
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  
  release(&cons.lock);

  int bytes_read = target - n;
  if(DEBUG_CONSOLE) {
    printf("[DEBUG] consoleread: read %d bytes\n", bytes_read);
  }

  return bytes_read;
}

//
// The console input interrupt handler.
// uartintr() calls this for input character.
// Do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int c)
{
  acquire(&cons.lock);

  if(DEBUG_INPUT) {
    printf("[DEBUG] consoleintr: received char 0x%x ('%c')\n", 
           c, (c >= 32 && c < 127) ? c : '?');
  }

  switch(c){
  case C('P'):  // Print process list.
    procdump();
    break;
    
  case C('U'):  // Kill line.
    while(cons.e != cons.w &&
          cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
    
  case C('H'): // Backspace
  case '\x7f': // Delete key
    if(cons.e != cons.w){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
    
  default:
    if(c != 0 && cons.e - cons.r < INPUT_BUF_SIZE){
      c = (c == '\r') ? '\n' : c;

      // echo back to the user.
      consputc(c);

      // store for consumption by consoleread().
      cons.buf[cons.e++ % INPUT_BUF_SIZE] = c;

      if(c == '\n' || c == C('D') || cons.e - cons.r == INPUT_BUF_SIZE){
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        cons.w = cons.e;
        wakeup(&cons.r);
      }
    } else if(cons.e - cons.r >= INPUT_BUF_SIZE) {
      // Buffer full, drop character but warn
      if(DEBUG_INPUT) {
        printf("[WARNING] consoleintr: input buffer full, dropping char\n");
      }
    }
    break;
  }
  
  release(&cons.lock);
}

void
consoleinit(void)
{
  initlock(&cons.lock, "cons");

  // Initialize buffer indices
  cons.r = 0;
  cons.w = 0;
  cons.e = 0;

  uartinit();

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
  
  if(DEBUG_CONSOLE) {
    printf("[DEBUG] consoleinit: console initialized\n");
  }
}