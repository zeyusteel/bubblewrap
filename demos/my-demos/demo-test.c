#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <sys/prctl.h>
#include <syslog.h>
#include <sys/poll.h>
#include <sys/signalfd.h>

static void
block_sigchild (void)
{
  sigset_t mask;
  int status;

  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);

  sigprocmask (SIG_BLOCK, &mask, NULL);

  /* Reap any outstanding zombies that we may have inherited */
  while (waitpid (-1, &status, WNOHANG) > 0)
    ;
}

static void
unblock_sigchild (void)
{
  sigset_t mask;

  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);

  sigprocmask (SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char const *argv[])
{
    pid_t pid;

    int pipe_info[2];
    int pipe_fork[2];
    int json_status[2];
    int sync_pipe[2];

    char * args[30] = {0};
    int i = 0;

    char buf[1024] = {0};

    pipe(pipe_info);
    pipe(pipe_fork);
    pipe(json_status);
    pipe(sync_pipe);

    block_sigchild();

    //prctl (PR_SET_CHILD_SUBREAPER, 1, 0, 0, 0);

    for (; i < 2; i++) {
        // if (i == 1) continue;
        pid = fork();
        if (pid == 0) {
            break;
        } 
    }

    if (i == 2) {
        printf("parent %d\n", getpid());

        struct pollfd fd[3];
        struct signalfd_siginfo fdsi;
        int sigfd;
        int rc;
        pid_t ggpid;
        
        
        sigset_t mask;


        close(pipe_fork[0]);
        close(pipe_fork[1]);

        close(pipe_info[1]);
        read(pipe_info[0], buf, 1024);

        printf("%s\n", buf);
        close(pipe_info[0]);

        memset(buf, 0, sizeof(buf));

        close(json_status[1]);
        read(json_status[0], buf, 1024);

        printf("%s\n", buf);

        
        sigemptyset (&mask);
        sigaddset (&mask, SIGCHLD);

        sigfd = signalfd(-1, &mask, SFD_NONBLOCK);
        if (sigfd == -1) {
            printf("signalfd %s\n", strerror(errno));
            exit(-1);
        }

        fd[0].events = POLLIN;
        fd[0].fd = sigfd;
        fd[0].revents = -1;

        while(1) {
            rc = poll(fd, 1, -1);    
            if (rc == -1 && errno != EINTR) {
                printf("poll %s\n", strerror(errno));
                exit(-1);
            }

            if (rc == 1 && fd[0].revents == POLLIN) {
                
                rc = read (sigfd, &fdsi, sizeof (struct signalfd_siginfo));
                if (rc == -1 && errno != EINTR && errno != EAGAIN) {
                    printf("read %s\n", strerror(errno));                    
                }
                
                while ((ggpid = waitpid (-1, NULL, WNOHANG)) > 0) {
                    printf("catch %d\n", ggpid);
                }
                if (ggpid == -1) {
                    break;
                }
            }
        }
    
        close(sync_pipe[1]);
        printf("read\n");
        if (read(sync_pipe[0], buf, 1) == 0) {
            printf("test sync pipe close\n");
        }

        memset(buf, 0, sizeof(buf));
        read(json_status[0], buf, 1024);

        printf("%s\n", buf);


    } else if (i == 0) {
        printf("child %d\n", getpid());
        unblock_sigchild();

        close(pipe_fork[0]);
        close(pipe_fork[1]);

        close(pipe_info[0]);
        close(pipe_info[1]);

        close(json_status[0]);
        close(json_status[1]);


        close(sync_pipe[0]);
        close(sync_pipe[1]);

    } else if (i == 1) {
        printf("child %d\n", getpid());
        unblock_sigchild();

        close(pipe_fork[1]);
        close(pipe_fork[0]);


        close(pipe_info[0]);
        close(json_status[0]);

        close(sync_pipe[0]);

        char buf[1024] = {0};

        args[0] = "bwrap";
        args[1] = "--dev-bind";
        args[2] = "/";
        args[3] = "/";

        args[4] = "--unshare-all";
        args[5] = "--unshare-user";

        args[6] = "--info-fd";
        snprintf(buf, sizeof(buf), "%d", pipe_info[1]);
        args[7] = strdup(buf);

        memset(buf, 0, sizeof(buf));

        args[8] = "--json-status-fd";
        snprintf(buf, sizeof(buf), "%d", json_status[1]);
        args[9] = strdup(buf);

        args[10] = "--sync-fd";
        snprintf(buf, sizeof(buf), "%d", sync_pipe[1]);
        args[11] = strdup(buf);

        args[12] = "--as-pid-1";

        args[13] = "bash";

        int rc = execvp("bwrap", args);
        if (rc == -1) {
            printf("execvp: %s\n", strerror(errno));
        }
    }

    printf("exit %d\n", getpid());
    return 0;
}
