#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Create an orphaned file and check if test-xv6.py recovers it.

int
main(int argc, char **argv)
{
  int fd = 0;
  char *s = argv[0];
  struct stat st;
  char *ff = "file0";
  
  // Create file
  if ((fd = open(ff, O_CREATE|O_WRONLY)) < 0) {
    printf("%s: open failed\n", s);
    exit(1);
  }
  
  // Get file stats
  if(fstat(fd, &st) < 0){
    fprintf(2, "%s: cannot stat %s\n", s, ff);  // Fixed: use ff instead of "ff"
    close(fd);
    exit(1);
  }
  
  // Unlink the file while we still have it open
  if (unlink(ff) < 0) {
    printf("%s: unlink failed\n", s);
    close(fd);
    exit(1);
  }
  
  // Verify file is really unlinked
  if (open(ff, O_RDONLY) >= 0) {  // Fixed: changed != -1 to >= 0
    printf("%s: open succeeded when it should have failed\n", s);
    close(fd);
    exit(1);
  }
  
  printf("wait for kill and reclaim %d\n", st.ino);
  
  // Sit around until killed - file remains orphaned
  for(;;) {
    pause(1000);
  }
}