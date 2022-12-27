#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

int
raw_clone (unsigned long flags,
           void         *child_stack)
{
#if defined(__s390__) || defined(__CRIS__)
  /* On s390 and cris the order of the first and second arguments
   * of the raw clone() system call is reversed. */
  return (int) syscall (__NR_clone, child_stack, flags);
#else
  return (int) syscall (__NR_clone, flags, child_stack);
#endif
}

void print_all_cap();

int main(int argc, char const *argv[])
{
    int pid;
    printf("before clone uid: %d  euid: %d\n", getuid(), geteuid());
    print_all_cap();

    pid = raw_clone(CLONE_NEWUSER | SIGCHLD, NULL);
    if (pid == -1) {
        fprintf(stderr, "raw_clone:, %s\n",strerror(errno));
		exit(EXIT_FAILURE);
    } else if (pid > 0) {
    	printf("parent %d \n", getpid());
        waitpid(pid, NULL, 0);
    } else {
        setuid(1001); 
        perror("wtf");
    	printf("child %d uid: %d euid %d\n", getpid(), getuid(), geteuid());
        print_all_cap();
    }

    return 0;
}