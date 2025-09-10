#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    fprintf(2, "usage: kill pid...\n");
    exit(1);
  }
  
  for(i = 1; i < argc; i++){
    int pid = atoi(argv[i]);
    if(pid <= 0) {
      fprintf(2, "kill: invalid pid %s\n", argv[i]);
      continue;
    }
    if(kill(pid) < 0){
      fprintf(2, "kill: %s failed\n", argv[i]);
    }
  }
  
  exit(0);
}