//
// Support functions for system calls that involve file descriptors.
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "proc.h"

// Debug flags
#define DEBUG_FILE 0
#define DEBUG_FILE_ALLOC 0

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
  
  // Initialize all file entries
  for(int i = 0; i < NFILE; i++) {
    ftable.file[i].ref = 0;
    ftable.file[i].type = FD_NONE;
    ftable.file[i].readable = 0;
    ftable.file[i].writable = 0;
    ftable.file[i].pipe = 0;
    ftable.file[i].ip = 0;
    ftable.file[i].off = 0;
    ftable.file[i].major = 0;
  }
  
  if(DEBUG_FILE) {
    printf("[DEBUG] fileinit: file table initialized with %d entries\n", NFILE);
  }
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      f->type = FD_NONE;  // Initialize type
      f->readable = 0;
      f->writable = 0;
      f->pipe = 0;
      f->ip = 0;
      f->off = 0;
      f->major = 0;
      release(&ftable.lock);
      
      if(DEBUG_FILE_ALLOC) {
        printf("[DEBUG] filealloc: allocated file %d\n", (int)(f - ftable.file));
      }
      return f;
    }
  }
  release(&ftable.lock);
  
  if(DEBUG_FILE_ALLOC) {
    printf("[ERROR] filealloc: no free file structures\n");
  }
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  if(!f) {
    if(DEBUG_FILE) {
      printf("[ERROR] filedup: null file pointer\n");
    }
    return 0;
  }

  acquire(&ftable.lock);
  if(f->ref < 1) {
    release(&ftable.lock);
    panic("filedup");
  }
  f->ref++;
  release(&ftable.lock);
  
  if(DEBUG_FILE) {
    printf("[DEBUG] filedup: file ref count now %d\n", f->ref);
  }
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  if(!f) {
    if(DEBUG_FILE) {
      printf("[ERROR] fileclose: null file pointer\n");
    }
    return;
  }

  acquire(&ftable.lock);
  if(f->ref < 1) {
    release(&ftable.lock);
    panic("fileclose");
  }
  
  if(--f->ref > 0){
    release(&ftable.lock);
    if(DEBUG_FILE) {
      printf("[DEBUG] fileclose: ref count now %d, not closing\n", f->ref);
    }
    return;
  }
  
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  f->readable = 0;
  f->writable = 0;
  f->pipe = 0;
  f->ip = 0;
  f->off = 0;
  f->major = 0;
  release(&ftable.lock);

  if(DEBUG_FILE) {
    printf("[DEBUG] fileclose: closing file type %d\n", ff.type);
  }

  if(ff.type == FD_PIPE){
    pipeclose(ff.pipe, ff.writable);
  } else if(ff.type == FD_INODE || ff.type == FD_DEVICE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
// addr is a user virtual address, pointing to a struct stat.
int
filestat(struct file *f, uint64 addr)
{
  struct proc *p = myproc();
  struct stat st;
  
  if(!f) {
    if(DEBUG_FILE) {
      printf("[ERROR] filestat: null file pointer\n");
    }
    return -1;
  }
  
  if(f->type == FD_INODE || f->type == FD_DEVICE){
    if(!f->ip) {
      if(DEBUG_FILE) {
        printf("[ERROR] filestat: null inode\n");
      }
      return -1;
    }
    
    ilock(f->ip);
    stati(f->ip, &st);
    iunlock(f->ip);
    
    if(copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0) {
      if(DEBUG_FILE) {
        printf("[ERROR] filestat: copyout failed\n");
      }
      return -1;
    }
    
    if(DEBUG_FILE) {
      printf("[DEBUG] filestat: success type=%d size=%d\n", st.type, st.size);
    }
    return 0;
  }
  
  if(DEBUG_FILE) {
    printf("[ERROR] filestat: unsupported file type %d\n", f->type);
  }
  return -1;
}

// Read from file f.
// addr is a user virtual address.
int
fileread(struct file *f, uint64 addr, int n)
{
  int r = 0;

  if(!f) {
    if(DEBUG_FILE) {
      printf("[ERROR] fileread: null file pointer\n");
    }
    return -1;
  }

  if(f->readable == 0) {
    if(DEBUG_FILE) {
      printf("[ERROR] fileread: file not readable\n");
    }
    return -1;
  }

  if(n < 0) {
    if(DEBUG_FILE) {
      printf("[ERROR] fileread: negative byte count %d\n", n);
    }
    return -1;
  }

  if(n == 0) {
    return 0;
  }

  if(DEBUG_FILE) {
    printf("[DEBUG] fileread: type=%d addr=0x%p n=%d\n", f->type, addr, n);
  }

  if(f->type == FD_PIPE){
    r = piperead(f->pipe, addr, n);
  } else if(f->type == FD_DEVICE){
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read) {
      if(DEBUG_FILE) {
        printf("[ERROR] fileread: invalid device major %d\n", f->major);
      }
      return -1;
    }
    r = devsw[f->major].read(1, addr, n);
  } else if(f->type == FD_INODE){
    if(!f->ip) {
      if(DEBUG_FILE) {
        printf("[ERROR] fileread: null inode\n");
      }
      return -1;
    }
    ilock(f->ip);
    if((r = readi(f->ip, 1, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
  } else {
    if(DEBUG_FILE) {
      printf("[ERROR] fileread: unknown file type %d\n", f->type);
    }
    panic("fileread");
  }

  if(DEBUG_FILE && r >= 0) {
    printf("[DEBUG] fileread: read %d bytes\n", r);
  }

  return r;
}

// Write to file f.
// addr is a user virtual address.
int
filewrite(struct file *f, uint64 addr, int n)
{
  int r, ret = 0;

  if(!f) {
    if(DEBUG_FILE) {
      printf("[ERROR] filewrite: null file pointer\n");
    }
    return -1;
  }

  if(f->writable == 0) {
    if(DEBUG_FILE) {
      printf("[ERROR] filewrite: file not writable\n");
    }
    return -1;
  }

  if(n < 0) {
    if(DEBUG_FILE) {
      printf("[ERROR] filewrite: negative byte count %d\n", n);
    }
    return -1;
  }

  if(n == 0) {
    return 0;
  }

  if(DEBUG_FILE) {
    printf("[DEBUG] filewrite: type=%d addr=0x%p n=%d\n", f->type, addr, n);
  }

  if(f->type == FD_PIPE){
    ret = pipewrite(f->pipe, addr, n);
  } else if(f->type == FD_DEVICE){
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].write) {
      if(DEBUG_FILE) {
        printf("[ERROR] filewrite: invalid device major %d\n", f->major);
      }
      return -1;
    }
    ret = devsw[f->major].write(1, addr, n);
  } else if(f->type == FD_INODE){
    // Write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * BSIZE;
    int i = 0;
    
    if(!f->ip) {
      if(DEBUG_FILE) {
        printf("[ERROR] filewrite: null inode\n");
      }
      return -1;
    }
    
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r != n1){
        // error from writei
        if(DEBUG_FILE) {
          printf("[ERROR] filewrite: writei failed, wrote %d/%d bytes\n", r, n1);
        }
        break;
      }
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    if(DEBUG_FILE) {
      printf("[ERROR] filewrite: unknown file type %d\n", f->type);
    }
    panic("filewrite");
  }

  if(DEBUG_FILE && ret >= 0) {
    printf("[DEBUG] filewrite: wrote %d bytes\n", ret);
  }

  return ret;
}