#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Create an orphaned directory and check if test-xv6.py recovers it.

int
main(int argc, char **argv)
{
  char *s = argv[0];

  if(mkdir("dd") != 0){
    printf("%s: mkdir dd failed\n", s);
    exit(1);
  }

  if(chdir("dd") != 0){
    printf("%s: chdir dd failed\n", s);
    exit(1);
  }

  // Remove the directory while we're still inside it
  if (unlink("../dd") < 0) {
    printf("%s: unlink failed\n", s);
    exit(1);
  }
  
  printf("wait for kill and reclaim\n");
  
  // Sit around until killed - creates orphaned directory scenario
  for(;;) {
    pause(1000);
  }
}