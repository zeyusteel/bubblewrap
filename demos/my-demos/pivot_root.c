/* pivot_root_demo.c */

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/mman.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
/*

pivot_root切换当前进程所在mount namespace的root mount,旧的root mount会被放到put_old
然后将new_root作为新的root mount，调用者需要在当前命名空间有 CAP_SYS_ADMIN能力

pivot_root会将当前同一mount namespace所有进程，线程的cwd都切换到new_root
但是pivot_root不会切换调用进程的cwd,除它的cwd在old root的 /
因此一般都需要配合chdir("/")使用

限定
new_root 和 put_old 都是目录
new_root 和 put_old 不能在同一个挂载点
put_old 必须在 new_root的根或子目录
new_root 必须是挂载点，并且不能是 "/", 通过 bind mount的方式将目录bind到自己身上可以把自己变成挂载点
new_root的parent mount和当前根的parent mount传播类型不能是 MS_SHARED,如果put_old也是挂载点，那么传播类型也不能是MS_SHARED,这些限制确保pivot_root（）从不将任何更改传播到另一个mount namespace
当前的根目录必须是挂载点

pivot_root(".", ".")
    新旧root是同一个目录，这时就不需要创建临时文件处理put_old

    chdir(new_root);
    pivot_root(".", ".");
    umount2(".", MNT_DETACH);

    这时old_root会叠加在new_root之上,只需要umount . 就会把old_root卸载


*/
static int
pivot_root(const char *new_root, const char *put_old)
{
    return syscall(SYS_pivot_root, new_root, put_old);
}

#define STACK_SIZE (1024 * 1024)

static int              /* Startup function for cloned child */
child(void *arg)
{
    char **args = arg;
    char *new_root = args[0];
    const char *put_old = "/oldrootfs";
    char path[PATH_MAX];

    /* Ensure that 'new_root' and its parent mount don't have
       shared propagation (which would cause pivot_root() to
       return an error), and prevent propagation of mount
       events to the initial mount namespace */

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == 1)
        errExit("mount-MS_PRIVATE");

    /* Ensure that 'new_root' is a mount point */

    if (mount(new_root, new_root, NULL, MS_BIND, NULL) == -1)
        errExit("mount-MS_BIND");

    /* Create directory to which old root will be pivoted */

    snprintf(path, sizeof(path), "%s/%s", new_root, put_old);
    if (mkdir(path, 0777) == -1)
        errExit("mkdir");

    /* And pivot the root filesystem */

    if (pivot_root(new_root, path) == -1)
        errExit("pivot_root");

    /* Switch the current working directory to "/" */

    if (chdir("/") == -1)
        errExit("chdir");

    /* Unmount old root and remove mount point */

    if (umount2(put_old, MNT_DETACH) == -1)
        perror("umount2");
    if (rmdir(put_old) == -1)
        perror("rmdir");

    /* Execute the command specified in argv[1]... */

    execv(args[1], &args[1]);
    errExit("execv");
}

int
main(int argc, char *argv[])
{
    /* Create a child process in a new mount namespace */

    char *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");

    if (clone(child, stack + STACK_SIZE,
                CLONE_NEWNS | SIGCHLD, &argv[1]) == -1)
        errExit("clone");

    /* Parent falls through to here; wait for child */

    if (wait(NULL) == -1)
        errExit("wait");

    exit(EXIT_SUCCESS);
}