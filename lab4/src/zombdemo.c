#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
  pid_t pid = fork();

  if (pid == 0) {
    //потомок сразу завершается
    printf("[child] pid=%d, exiting\n", getpid());
    return 0;
  }

  // родитель не вызывает wait
  printf("[parent] pid=%d, child pid=%d will be zombie\n",
         getpid(), pid);
  printf("[parent] sleep 20s\n");
  sleep(20);

  return 0;
}