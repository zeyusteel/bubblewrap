#include <sys/capability.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <string.h>
#include <errno.h>
 
 
#define nitems(x) (sizeof(x) / sizeof(x[0]))
 
static const char *_cap_name[] = {
    "cap_chown",
    "cap_dac_override",
    "cap_dac_read_search",
    "cap_fowner",
    "cap_fsetid",
    "cap_kill",
    "cap_setgid",
    "cap_setuid",
    "cap_setpcap",
    "cap_linux_immutable",
    "cap_net_bind_service",
    "cap_net_broadcast",
    "cap_net_admin",
    "cap_net_raw",
    "cap_ipc_lock",
    "cap_ipc_owner",
    "cap_sys_module",
    "cap_sys_rawio",
    "cap_sys_chroot",
    "cap_sys_ptrace",
    "cap_sys_pacct",
    "cap_sys_admin",
    "cap_sys_boot",
    "cap_sys_nice",
    "cap_sys_resource",
    "cap_sys_time",
    "cap_sys_tty_config",
    "cap_mknod",
    "cap_lease",
    "cap_audit_write",
    "cap_audit_control",
    "cap_setfcap",
    "cap_mac_override",
    "cap_mac_admin",
    "cap_syslog",
    "cap_wake_alarm",
    "cap_block_suspend",
    "cap_audit_read",
    NULL
};
static const int _cap_size = nitems(_cap_name) - 1;
 
static void _cap_eip_dump(cap_t cap)
{
    cap_value_t cap_list[CAP_LAST_CAP+1];
    cap_flag_value_t cap_flags_value;
    /* temporary use for cap_get_flag calls */
    struct {
        const char *str;
        cap_flag_t flag;
    } const flags[3] = {
        {"EFFECTIVE", CAP_EFFECTIVE},
        {"PERMITTED", CAP_PERMITTED},
        {"INHERITABLE", CAP_INHERITABLE}
    };
    int i, j;
 
    /* dump them */
    for (i=0; i < _cap_size && i < CAP_LAST_CAP; i++) {
        cap_from_name(_cap_name[i], &cap_list[i]);
        printf("%-20s %d\t\t", cap_to_name(cap_list[i]), cap_list[i]);
        printf("flags: \t\t");
        for (j=0; j < nitems(flags); j++) {
            cap_get_flag(cap, cap_list[i], flags[j].flag, &cap_flags_value);
            printf(" %s %-4s ", flags[j].str, (cap_flags_value == CAP_SET) ? "OK" : "NOK");
        }
        printf("\n");
    }
    printf("\n");
}
 
static void cap_eip_dump(void)
{
    pid_t pid;
    cap_t cap;
 
    pid = getpid();
    cap = cap_get_pid(pid);
    if (cap == NULL) {
        perror("cap_get_pid");
        exit(-1);
    }
 
    /* dump them */
    printf("dump proc capabilities:\n");
    _cap_eip_dump(cap);
 
    cap_free(cap);
}
 
 
static int _cap_eip_set(const cap_flag_t *cap_flag, int cap_flag_size,
                    const cap_value_t *cap_list, int cap_list_size,
                    int set)
{
    cap_t cap;
    int i;
 
    for (i = 0; i < cap_flag_size; ++i)
    {
        if (cap_flag[i] != CAP_EFFECTIVE
                && cap_flag[i] != CAP_PERMITTED
                && cap_flag[i] != CAP_INHERITABLE)
        {
            perror("invalid argument");
            return -1;
        }
    }
    for (i = 0; i < cap_list_size; ++i)
    {
        if (cap_list[i] > CAP_LAST_CAP)
        {
            perror("invalid argument");
            return -1;
        }
    }
 
    //1. get cap
    cap = cap_get_proc();
    if (cap == NULL) {
        perror("cap_get_proc");
        return -1;
    }
 
    //2. set cap flag
    set = set ? CAP_SET : CAP_CLEAR;
    for (i = 0; i < cap_flag_size; ++i)
    {
        /* set CAP_EFFECTIVE/CAP_PERMITTED/CAP_INHERITABLE cap */
        if (cap_set_flag(cap, cap_flag[i], cap_list_size, cap_list, set) == -1) {
            perror("cap_set_flag CAP_KILL");
            cap_free(cap);
            return -1;
        }
    }
 
    //3. set cap to kernel
    if (cap_set_proc(cap) < 0) {
        perror("cap_set_proc fail");
        cap_free(cap);
        return -1;
    }
 
    //4. clean
    cap_free(cap);
 
    return 0;
}
static int cap_eip_set(const cap_flag_t *cap_flag, int cap_flag_size,
                    const cap_value_t *cap_list, int cap_list_size)
{
    return _cap_eip_set(cap_flag, cap_flag_size, cap_list, cap_list_size, 1);
}
 
static void cap_ambient_dump(void)
{
    cap_value_t cap_list[CAP_LAST_CAP+1];
    int value;
    int i;
 
    if (!CAP_AMBIENT_SUPPORTED()) {
        printf("kernel NOT support AMBIENT!\n");
        return;
    }
 
    printf("dump ambient capabilities:\n");
 
    /* dump them */
    for (i=0; i < _cap_size; i++) {
        cap_from_name(_cap_name[i], &cap_list[i]);
        value = cap_get_ambient(cap_list[i]);
        if (value < 0) {
            continue;
        }
 
        printf("%-20s %d\t\t", cap_to_name(cap_list[i]), cap_list[i]);
        printf("flags: \t\t");
        printf(" %s %-4s ", "AMBIENT", (value == CAP_SET) ? "OK" : "NOK");
        printf("\n");
    }
    printf("\n");
}
 
static int cap_ambient_set(const cap_value_t *cap_list, int cap_list_size)
{
    int i;
 
    for (i = 0; i < cap_list_size; ++i)
    {
        if (cap_list[i] > CAP_LAST_CAP)
        {
            perror("invalid argument");
            return -1;
        }
    }
 
    for (i = 0; i < cap_list_size; ++i)
    {
        if (cap_set_ambient(cap_list[i], CAP_SET) < 0) {
            perror("cap_set_ambient fail");
            return -1;
        }
    }
 
    return 0;
}
 
 
static int test_cap[] = {CAP_KILL};
static void child_runner(void *userp)
{
    char path[128] = {'\0'};
    char *arg[8] = {NULL};
    int ret;
 
    ret = readlink("/proc/self/exe", path, sizeof(path));
    if (ret) {
        //make compiler happy
    }
 
    arg[0] = path;
    arg[1] = "--test";
    arg[2] = userp;
    execv(path, arg);
}
static void child_handler(void *userp)
{
    /* 作为子进程被execv()所运行 */
    /* 验证execv()后进程是否还具有我们测试的CAP_KILL权限 */
    char *parent_pid = userp;
    printf("after execv(), I am child bin\r\n");
    cap_eip_dump();
    cap_ambient_dump();
    printf("========\n");
 
    sleep(1);
    kill(atoi(parent_pid), SIGINT);
}
 
int main(int argc, char *argv[])
{
    cap_value_t cap_list[CAP_LAST_CAP+1];
    cap_flag_t cap_flag[3];
    pid_t parent_pid;
    pid_t child_pid;
 
    if (argc > 2 && !strcmp(argv[1], "--test"))
    {
        child_handler(argv[2]);
        return 0;
    }
 
    parent_pid = getpid();
    child_pid = fork();
    if (child_pid < 0)
    {
        exit(-1);
    }
    else if (child_pid > 0)
    {
        //parent
        printf("I am parent, pid=%d, child pid=%d\n", parent_pid, child_pid);
        while (1)
        {
            sleep(1);
        }
    }
    else
    {
        //child
        int i;
 
        cap_eip_dump();
        cap_ambient_dump();
        printf("========\n");
 
        /* enable CAP_SETPCAP, or PR_SET_KEEPCAPS will fail */
        cap_flag[0] = CAP_EFFECTIVE;
        cap_flag[1] = CAP_PERMITTED;
        cap_flag[2] = CAP_INHERITABLE;
        memset(cap_list, 0, sizeof(cap_list));
        cap_list[0] = CAP_SETPCAP;
        cap_eip_set(cap_flag, 3, cap_list, 1);
 
        /* enable testing cap */
        /* 这里把我们想要测试的权限设置进去 */
        cap_flag[0] = CAP_EFFECTIVE;
        cap_flag[1] = CAP_PERMITTED;
        cap_flag[2] = CAP_INHERITABLE;
        memset(cap_list, 0, sizeof(cap_list));
        for (i = 0; i < nitems(test_cap); ++i) {
            cap_list[i] = test_cap[i];
        }
        cap_eip_set(cap_flag, 3, cap_list, nitems(test_cap));
 
        cap_eip_dump();
        cap_ambient_dump();
        printf("========\n");
 
        /*! keep caps after setuid */
        /* 必须设置PR_SET_KEEPCAPS, 否则调用setuid之后，所有权限会被清空 */
        prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
 
        setuid(1000);
 
        printf("after setuid\n");
        cap_eip_dump();
        cap_ambient_dump();
        printf("========\n");
 
        /* 运行execv()前，必须把我们想要的权限设置到ambient权限集中 */
        /* 否则运行execv()后，无法获得该权限 */
        memset(cap_list, 0, sizeof(cap_list));
        for (i = 0; i < nitems(test_cap); ++i) {
            cap_list[i] = test_cap[i];
        }
        cap_ambient_set(cap_list, nitems(test_cap));
 
        printf("before execv()\n");
        cap_eip_dump();
        cap_ambient_dump();
        printf("========\n");
 
        {
            char arg[16];
            snprintf(arg, sizeof(arg), "%d", parent_pid);
            child_runner(arg);
        }
    }
 
    return 0;
}