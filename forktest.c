#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N  1000

void
print(const char *s)
{
  write(1, s, strlen(s));
}

void
forktest(void)
{
  int n, pid;

  print("fork test\n");

  // Try to fork N times
  for(n = 0; n < N; n++){
    pid = fork();
    if(pid < 0)
      break;  // Fork failed
    if(pid == 0)
      exit(0);  // Child exits immediately
  }

  if(n == N){
    print("fork claimed to work N times!\n");
    exit(1);
  }

  // Wait for all children
  for(; n > 0; n--){
    if(wait(0) < 0){
      print("wait stopped early\n");
      exit(1);
    }
  }

  // Should not have any more children
  if(wait(0) != -1){
    print("wait got too many\n");
    exit(1);
  }

  print("fork test OK\n");
}

int
main(void)
{
  forktest();
  exit(0);
}