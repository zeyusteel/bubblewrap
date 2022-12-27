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

#if 1
  if (sigprocmask (SIG_BLOCK, &mask, NULL) == -1) {
      printf("%s \n",__func__);
    return;
  }
#endif
}

void test(int sig)
{
	printf("-----------------signal :%d\n", sig);
}

int main(int argc, char const *argv[])
{
    pid_t pid = fork();

    if (pid > 0) {
        signal(SIGCHLD, test);
        printf("i am parent %d\n", getpid());
        block_sigchild ();
        wait(NULL); //屏蔽SIGCHLD也不影响进程回收
        printf("parent exit\n");
        exit(0);
    } else {
        sleep(3);
        printf("i am child %d\n", getpid());
        printf("child exit\n");
        exit(0);
    }

    /* code */
    return 0;
}
