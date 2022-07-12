#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/fsuid.h>

int main(int argc, char const *argv[])
{
    /* code */
    uid_t uid = getuid();
    printf("%d\n", uid);
    uid_t euid = geteuid();
    printf("%d\n", euid);

    uid_t fs = setfsuid(-1);
    printf("fs:%d\n", fs);
    if (setfsuid(uid) < 0)
        perror(NULL);

    fs = setfsuid(-1);
    printf("fs:%d\n", fs);

    int rc = open(argv[1], O_RDWR);
    if (rc < 0)
        perror(NULL);
    
    write(rc, "abc", 3);
    return 0;
}
