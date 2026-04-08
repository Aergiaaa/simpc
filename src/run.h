#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int run(char *path, char **argv) {
  pid_t pid = fork();

  if (pid < 0) {
    printf("error fork");
    return -1;
  }

  // child
  if (pid == 0) {
    execve(path, argv, NULL);
    printf("error execve");
    _exit(127);
  }

  // parent
  int status;
  waitpid(pid, &status, 0);

  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
