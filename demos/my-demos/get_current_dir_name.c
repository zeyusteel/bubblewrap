#define	_GNU_SOURCE
#include <unistd.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    /* code */
    char buf[1024] = {0};
    getcwd(buf, 1024);
    printf("%s\n", buf);

    char *p = get_current_dir_name();
    printf("%s\n", p);
    return 0;
}
