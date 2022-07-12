#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void
block_sigchild (void)
{
  sigset_t mask;
  int status;

  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);

  if (sigprocmask (SIG_BLOCK, &mask, NULL) == -1) {
      printf("%s \n",__func__);
    return;
  }

  /* Reap any outstanding zombies that we may have inherited */
  while (waitpid (-1, &status, WNOHANG) > 0) {
      printf("wtf\n");
      ;
  }
}

int main(int argc, char const *argv[])
{
    pid_t pid = fork();

    if (pid > 0) {
        printf("parent %d\n", pid);
        sleep(1);
        block_sigchild ();
    } else {
        printf("i am child %d\n", getpid());
        exit(1);
    }

    /* code */
    return 0;
}
