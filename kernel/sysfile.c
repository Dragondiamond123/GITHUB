//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

// Debug configuration
#define DEBUG_FILE_SYSCALLS 1
#define DEBUG_FILE_DESCRIPTORS 1
#define DEBUG_PATH_RESOLUTION 1
#define DEBUG_INODE_OPS 1
#define DEBUG_SECURITY_CHECKS 1

// File operation statistics
static struct {
  uint64 open_count;
  uint64 close_count;
  uint64 read_count;
  uint64 write_count;
  uint64 link_count;
  uint64 unlink_count;
  uint64 mkdir_count;
  uint64 failed_ops;
  struct spinlock lock;
} file_stats;

// Initialize file system statistics
void
file_stats_init(void)
{
  initlock(&file_stats.lock, "file_stats");
  file_stats.open_count = 0;
  file_stats.close_count = 0;
  file_stats.read_count = 0;
  file_stats.write_count = 0;
  file_stats.link_count = 0;
  file_stats.unlink_count = 0;
  file_stats.mkdir_count = 0;
  file_stats.failed_ops = 0;
}

// Print file system statistics
void
print_file_stats(void)
{
  acquire(&file_stats.lock);
  printf("=== File System Statistics ===\n");
  printf("Opens: %d, Closes: %d\n", file_stats.open_count, file_stats.close_count);
  printf("Reads: %d, Writes: %d\n", file_stats.read_count, file_stats.write_count);
  printf("Links: %d, Unlinks: %d\n", file_stats.link_count, file_stats.unlink_count);
  printf("Mkdirs: %d\n", file_stats.mkdir_count);
  printf("Failed operations: %d\n", file_stats.failed_ops);
  release(&file_stats.lock);
}

// Debug function to validate file descriptor
static int
validate_fd(int fd, struct proc *p)
{
  if(fd < 0 || fd >= NOFILE) {
    if(DEBUG_SECURITY_CHECKS) {
      printf("[SECURITY] Invalid fd %d from pid %d\n", fd, p->pid);
    }
    return 0;
  }
  
  if(p->ofile[fd] == 0) {
    if(DEBUG_FILE_DESCRIPTORS) {
      printf("[ERROR] fd %d not open for pid %d\n", fd, p->pid);
    }
    return 0;
  }
  
  return 1;
}

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;
  struct proc *p = myproc();

  argint(n, &fd);
  
  if(DEBUG_FILE_DESCRIPTORS) {
    printf("[DEBUG] argfd: pid=%d checking fd=%d\n", p->pid, fd);
  }
  
  if(!validate_fd(fd, p)) {
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  
  f = p->ofile[fd];
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
    
  if(DEBUG_FILE_DESCRIPTORS) {
    printf("[DEBUG] argfd: pid=%d fd=%d -> file=0x%p type=%d\n", 
           p->pid, fd, f, f ? f->type : -1);
  }
  
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *p = myproc();

  if(DEBUG_FILE_DESCRIPTORS) {
    printf("[DEBUG] fdalloc: pid=%d looking for free fd\n", p->pid);
  }

  for(fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd] == 0){
      p->ofile[fd] = f;
      if(DEBUG_FILE_DESCRIPTORS) {
        printf("[DEBUG] fdalloc: pid=%d allocated fd=%d for file=0x%p\n", 
               p->pid, fd, f);
      }
      return fd;
    }
  }
  
  if(DEBUG_FILE_DESCRIPTORS) {
    printf("[ERROR] fdalloc: pid=%d no free file descriptors\n", p->pid);
  }
  
  acquire(&file_stats.lock);
  file_stats.failed_ops++;
  release(&file_stats.lock);
  
  return -1;
}

uint64
sys_dup(void)
{
  struct file *f;
  int fd;
  struct proc *p = myproc();

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] dup: pid=%d\n", p->pid);
  }

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] dup: pid=%d success, new fd=%d\n", p->pid, fd);
  }
  
  return fd;
}

uint64
sys_read(void)
{
  struct file *f;
  int n;
  uint64 p;
  struct proc *proc = myproc();

  acquire(&file_stats.lock);
  file_stats.read_count++;
  release(&file_stats.lock);

  argaddr(1, &p);
  argint(2, &n);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] read: pid=%d addr=0x%p n=%d\n", proc->pid, p, n);
  }
  
  // Security check: validate buffer size
  if(n < 0 || n > 1024*1024) {  // Limit to 1MB reads
    if(DEBUG_SECURITY_CHECKS) {
      printf("[SECURITY] read: pid=%d invalid size %d\n", proc->pid, n);
    }
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  
  if(argfd(0, 0, &f) < 0)
    return -1;
    
  int result = fileread(f, p, n);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] read: pid=%d result=%d\n", proc->pid, result);
  }
  
  return result;
}

uint64
sys_write(void)
{
  struct file *f;
  int n;
  uint64 p;
  struct proc *proc = myproc();
  
  acquire(&file_stats.lock);
  file_stats.write_count++;
  release(&file_stats.lock);

  argaddr(1, &p);
  argint(2, &n);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] write: pid=%d addr=0x%p n=%d\n", proc->pid, p, n);
  }
  
  // Security check: validate buffer size
  if(n < 0 || n > 1024*1024) {  // Limit to 1MB writes
    if(DEBUG_SECURITY_CHECKS) {
      printf("[SECURITY] write: pid=%d invalid size %d\n", proc->pid, n);
    }
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  
  if(argfd(0, 0, &f) < 0)
    return -1;

  int result = filewrite(f, p, n);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] write: pid=%d result=%d\n", proc->pid, result);
  }
  
  return result;
}

uint64
sys_close(void)
{
  int fd;
  struct file *f;
  struct proc *p = myproc();

  acquire(&file_stats.lock);
  file_stats.close_count++;
  release(&file_stats.lock);

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] close: pid=%d\n", p->pid);
  }

  if(argfd(0, &fd, &f) < 0)
    return -1;
    
  p->ofile[fd] = 0;
  fileclose(f);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] close: pid=%d closed fd=%d\n", p->pid, fd);
  }
  
  return 0;
}

uint64
sys_fstat(void)
{
  struct file *f;
  uint64 st; // user pointer to struct stat
  struct proc *p = myproc();

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] fstat: pid=%d\n", p->pid);
  }

  argaddr(1, &st);
  if(argfd(0, 0, &f) < 0)
    return -1;
    
  int result = filestat(f, st);
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] fstat: pid=%d result=%d\n", p->pid, result);
  }
  
  return result;
}

// Create the path new as a link to the same inode as old.
uint64
sys_link(void)
{
  char name[DIRSIZ], new[MAXPATH], old[MAXPATH];
  struct inode *dp, *ip;
  struct proc *p = myproc();

  acquire(&file_stats.lock);
  file_stats.link_count++;
  release(&file_stats.lock);

  if(argstr(0, old, MAXPATH) < 0 || argstr(1, new, MAXPATH) < 0) {
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] link: pid=%d old=\"%s\" new=\"%s\"\n", p->pid, old, new);
  }

  begin_op();
  if((ip = namei(old)) == 0){
    if(DEBUG_PATH_RESOLUTION) {
      printf("[ERROR] link: cannot find old path \"%s\"\n", old);
    }
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  ilock(ip);
  if(ip->type == T_DIR){
    if(DEBUG_SECURITY_CHECKS) {
      printf("[SECURITY] link: attempt to link directory \"%s\"\n", old);
    }
    iunlockput(ip);
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] link: pid=%d success\n", p->pid);
  }

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  acquire(&file_stats.lock);
  file_stats.failed_ops++;
  release(&file_stats.lock);
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

uint64
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], path[MAXPATH];
  uint off;
  struct proc *p = myproc();

  acquire(&file_stats.lock);
  file_stats.unlink_count++;
  release(&file_stats.lock);

  if(argstr(0, path, MAXPATH) < 0) {
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] unlink: pid=%d path=\"%s\"\n", p->pid, path);
  }

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    if(DEBUG_PATH_RESOLUTION) {
      printf("[ERROR] unlink: cannot find parent of \"%s\"\n", path);
    }
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0) {
    if(DEBUG_SECURITY_CHECKS) {
      printf("[SECURITY] unlink: attempt to unlink \"%s\"\n", name);
    }
    goto bad;
  }

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    if(DEBUG_FILE_SYSCALLS) {
      printf("[ERROR] unlink: directory \"%s\" not empty\n", path);
    }
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] unlink: pid=%d success\n", p->pid);
  }

  return 0;

bad:
  iunlockput(dp);
  end_op();
  acquire(&file_stats.lock);
  file_stats.failed_ops++;
  release(&file_stats.lock);
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if(DEBUG_INODE_OPS) {
    printf("[DEBUG] create: path=\"%s\" type=%d major=%d minor=%d\n", 
           path, type, major, minor);
  }

  if((dp = nameiparent(path, name)) == 0)
    return 0;

  ilock(dp);

  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE)) {
      if(DEBUG_INODE_OPS) {
        printf("[DEBUG] create: file already exists, returning existing inode\n");
      }
      return ip;
    }
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0){
    iunlockput(dp);
    return 0;
  }

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      goto fail;
  }

  if(dirlink(dp, name, ip->inum) < 0)
    goto fail;

  if(type == T_DIR){
    // now that success is guaranteed:
    dp->nlink++;  // for ".."
    iupdate(dp);
  }

  iunlockput(dp);

  if(DEBUG_INODE_OPS) {
    printf("[DEBUG] create: success, created inode %d\n", ip->inum);
  }

  return ip;

 fail:
  // something went wrong. de-allocate ip.
  if(DEBUG_INODE_OPS) {
    printf("[ERROR] create: failed to create \"%s\"\n", path);
  }
  ip->nlink = 0;
  iupdate(ip);
  iunlockput(ip);
  iunlockput(dp);
  return 0;
}

uint64
sys_open(void)
{
  char path[MAXPATH];
  int fd, omode;
  struct file *f;
  struct inode *ip;
  int n;
  struct proc *p = myproc();

  acquire(&file_stats.lock);
  file_stats.open_count++;
  release(&file_stats.lock);

  argint(1, &omode);
  if((n = argstr(0, path, MAXPATH)) < 0) {
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] open: pid=%d path=\"%s\" mode=0x%x\n", p->pid, path, omode);
  }

  // Security check: validate path length
  if(n <= 0 || n >= MAXPATH) {
    if(DEBUG_SECURITY_CHECKS) {
      printf("[SECURITY] open: invalid path length %d\n", n);
    }
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      acquire(&file_stats.lock);
      file_stats.failed_ops++;
      release(&file_stats.lock);
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      if(DEBUG_PATH_RESOLUTION) {
        printf("[ERROR] open: cannot find \"%s\"\n", path);
      }
      end_op();
      acquire(&file_stats.lock);
      file_stats.failed_ops++;
      release(&file_stats.lock);
      return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      if(DEBUG_SECURITY_CHECKS) {
        printf("[SECURITY] open: attempt to write to directory \"%s\"\n", path);
      }
      iunlockput(ip);
      end_op();
      acquire(&file_stats.lock);
      file_stats.failed_ops++;
      release(&file_stats.lock);
      return -1;
    }
  }

  if(ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV)){
    iunlockput(ip);
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }

  if(ip->type == T_DEVICE){
    f->type = FD_DEVICE;
    f->major = ip->major;
  } else {
    f->type = FD_INODE;
    f->off = 0;
  }
  f->ip = ip;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  if((omode & O_TRUNC) && ip->type == T_FILE){
    itrunc(ip);
  }

  iunlock(ip);
  end_op();

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] open: pid=%d success, fd=%d\n", p->pid, fd);
  }

  return fd;
}

uint64
sys_mkdir(void)
{
  char path[MAXPATH];
  struct inode *ip;
  struct proc *p = myproc();

  acquire(&file_stats.lock);
  file_stats.mkdir_count++;
  release(&file_stats.lock);

  begin_op();
  if(argstr(0, path, MAXPATH) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
    if(DEBUG_FILE_SYSCALLS) {
      printf("[ERROR] mkdir: pid=%d failed to create \"%s\"\n", p->pid, path);
    }
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] mkdir: pid=%d created \"%s\"\n", p->pid, path);
  }
  
  iunlockput(ip);
  end_op();
  return 0;
}

uint64
sys_mknod(void)
{
  struct inode *ip;
  char path[MAXPATH];
  int major, minor;
  struct proc *p = myproc();

  begin_op();
  argint(1, &major);
  argint(2, &minor);
  if((argstr(0, path, MAXPATH)) < 0 ||
     (ip = create(path, T_DEVICE, major, minor)) == 0){
    if(DEBUG_FILE_SYSCALLS) {
      printf("[ERROR] mknod: pid=%d failed to create device \"%s\"\n", p->pid, path);
    }
    end_op();
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] mknod: pid=%d created device \"%s\" major=%d minor=%d\n", 
           p->pid, path, major, minor);
  }
  
  iunlockput(ip);
  end_op();
  return 0;
}

uint64
sys_chdir(void)
{
  char path[MAXPATH];
  struct inode *ip;
  struct proc *p = myproc();
  
  begin_op();
  if(argstr(0, path, MAXPATH) < 0 || (ip = namei(path)) == 0){
    if(DEBUG_FILE_SYSCALLS) {
      printf("[ERROR] chdir: pid=%d cannot find \"%s\"\n", p->pid, path);
    }
    end_op();
    return -1;
  }
  ilock(ip);
  if(ip->type != T_DIR){
    if(DEBUG_FILE_SYSCALLS) {
      printf("[ERROR] chdir: pid=%d \"%s\" is not a directory\n", p->pid, path);
    }
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(p->cwd);
  end_op();
  p->cwd = ip;
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] chdir: pid=%d changed to \"%s\"\n", p->pid, path);
  }
  
  return 0;
}

uint64
sys_exec(void)
{
  char path[MAXPATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;
  struct proc *p = myproc();

  argaddr(1, &uargv);
  if(argstr(0, path, MAXPATH) < 0) {
    return -1;
  }
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] exec: pid=%d path=\"%s\"\n", p->pid, path);
  }
  
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      if(DEBUG_SECURITY_CHECKS) {
        printf("[SECURITY] exec: too many arguments (%d)\n", i);
      }
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = kexec(path, argv);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] exec: pid=%d result=%d\n", p->pid, ret);
  }

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  acquire(&file_stats.lock);
  file_stats.failed_ops++;
  release(&file_stats.lock);
  return -1;
}

uint64
sys_pipe(void)
{
  uint64 fdarray; // user pointer to array of two integers
  struct file *rf, *wf;
  int fd0, fd1;
  struct proc *p = myproc();

  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] pipe: pid=%d\n", p->pid);
  }

  argaddr(0, &fdarray);
  if(pipealloc(&rf, &wf) < 0) {
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      p->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  if(copyout(p->pagetable, fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
     copyout(p->pagetable, fdarray+sizeof(fd0), (char *)&fd1, sizeof(fd1)) < 0){
    p->ofile[fd0] = 0;
    p->ofile[fd1] = 0;
    fileclose(rf);
    fileclose(wf);
    acquire(&file_stats.lock);
    file_stats.failed_ops++;
    release(&file_stats.lock);
    return -1;
  }
  
  if(DEBUG_FILE_SYSCALLS) {
    printf("[SYSCALL] pipe: pid=%d success, fds=%d,%d\n", p->pid, fd0, fd1);
  }
  
  return 0;
}