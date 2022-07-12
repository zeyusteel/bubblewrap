#include <sys/eventfd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(int argc, char const *argv[])
{

    int child_wait_fd = eventfd (0, EFD_CLOEXEC);
    if (child_wait_fd == -1) {
        return -1;
    }

    pid_t pid = fork();
    uint64_t sig;

    if (pid == 0) {
        printf("before read\n");
        int rc = read(child_wait_fd, &sig, 8);    //没有收到信号默认阻塞, 除非fd设置 EFD_NONBLOCK
        printf("i am child :%d\n", getpid());
        printf("rc = %d, recv %ld\n", rc, sig);
        close(child_wait_fd);
    } else if (pid > 0) {
        sleep(3);
        sig = 1;
        int rc = write(child_wait_fd, &sig, 8);    //eventfd 收发一个64位无符号整形,发送小于8字节会报错
        printf("%d   -- %s\n", rc, strerror(errno));

        waitpid(pid, NULL, 0);
        close(child_wait_fd);
    }

    return 0;
}
