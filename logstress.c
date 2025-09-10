#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf[512];

//
// Stress test the logging system by doing lots of
// file system operations that cause logging activity.
//

void
logstress(void)
{
  int fd;
  int i, j;
  char fname[32];  // Increased buffer size for safety

  printf("logstress: start\n");

  // Create and delete many files quickly to stress the log
  for(i = 0; i < 100; i++){
    for(j = 0; j < 10; j++){
      // Use snprintf for safer string formatting
      if(snprintf(fname, sizeof(fname), "logfile%d", j) >= sizeof(fname)) {
        printf("logstress: filename too long\n");
        exit(1);
      }
      
      // Create file
      fd = open(fname, O_CREATE|O_RDWR);
      if(fd < 0){
        printf("logstress: create %s failed\n", fname);
        exit(1);
      }
      
      // Write some data
      if(write(fd, "hello world", 11) != 11){
        printf("logstress: write failed\n");
        close(fd);
        exit(1);
      }
      
      close(fd);
      
      // Delete file
      if(unlink(fname) < 0){
        printf("logstress: unlink %s failed\n", fname);
        exit(1);
      }
    }
    
    if(i % 10 == 0){
      printf("logstress: iteration %d\n", i);
    }
  }

  // Test directory operations
  for(i = 0; i < 50; i++){
    if(snprintf(fname, sizeof(fname), "logdir%d", i) >= sizeof(fname)) {
      printf("logstress: dirname too long\n");
      exit(1);
    }
    
    if(mkdir(fname) < 0){
      printf("logstress: mkdir %s failed\n", fname);
      exit(1);
    }
    
    // Create a file in the directory
    char fpath[64];
    if(snprintf(fpath, sizeof(fpath), "%s/file", fname) >= sizeof(fpath)) {
      printf("logstress: filepath too long\n");
      exit(1);
    }
    
    fd = open(fpath, O_CREATE|O_RDWR);
    if(fd < 0){
      printf("logstress: create %s failed\n", fpath);
      exit(1);
    }
    
    if(write(fd, "test", 4) != 4) {
      printf("logstress: write to %s failed\n", fpath);
      close(fd);
      exit(1);
    }
    close(fd);
    
    // Remove file and directory
    if(unlink(fpath) < 0){
      printf("logstress: unlink %s failed\n", fpath);
      exit(1);
    }
    
    if(unlink(fname) < 0){  // Remove directory
      printf("logstress: rmdir %s failed\n", fname);
      exit(1);
    }
  }

  printf("logstress: done\n");
}

int
main(int argc, char *argv[])
{
  int nproc = 1;
  int pid;
  int i;

  if(argc > 1){
    nproc = atoi(argv[1]);
    if(nproc <= 0 || nproc > 10){
      printf("logstress: invalid nproc %d\n", nproc);
      exit(1);
    }
  }

  printf("logstress: starting %d processes\n", nproc);

  for(i = 0; i < nproc; i++){
    pid = fork();
    if(pid < 0){
      printf("logstress: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      logstress();
      exit(0);
    }
  }

  // Wait for all child processes
  for(i = 0; i < nproc; i++){
    int status;
    wait(&status);
    if(status != 0) {
      printf("logstress: child exited with status %d\n", status);
    }
  }

  printf("logstress: all processes done\n");
  exit(0);
}