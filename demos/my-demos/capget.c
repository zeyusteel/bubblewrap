#undef _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/capability.h>
#include <errno.h>

#define nitems(x) (sizeof(x) / sizeof(x[0]))

//通过系统调用
static void print_cap_sys_call(void)
{
    struct __user_cap_header_struct cap_header_data;
    cap_user_header_t cap_header = &cap_header_data;

    struct __user_cap_data_struct cap_data_data[2] = {{0}, {0}};
    cap_user_data_t cap_data = cap_data_data;

    cap_header->pid = getpid();
    cap_header->version = _LINUX_CAPABILITY_VERSION_3;

    if (capget(cap_header, cap_data) < 0) {
        perror("Failed capget");
        exit(1);
    }
    printf("Cap data[0] 0x%08x, 0x%08x, 0x%08x\n", cap_data->effective,
        cap_data->permitted, cap_data->inheritable);


    printf("Cap data[1] 0x%08x, 0x%08x, 0x%08x\n", (cap_data+1)->effective,
        (cap_data+1)->permitted, (cap_data+1)->inheritable);
}

//通过libcap
static void print_cap_to_text(void)
{
    cap_t caps = cap_get_proc();
    ssize_t y = 0;
    printf("The process %d was give capabilities %s\n",
            (int) getpid(), cap_to_text(caps, &y));
    fflush(0);
    cap_free(caps);
}

//通过libcap
static void print_cap_get_flag(void)
{
    cap_t cap = cap_get_proc();
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
    int j;
    cap_value_t i;
 
    /* dump them */
    for (i=0; i < CAP_LAST_CAP; i++) {
        printf("%-20s %d\t\t", cap_to_name(i), i);
        printf("flags: \t\t");
        for (j=0; j < nitems(flags); j++) {
            cap_get_flag(cap, i, flags[j].flag, &cap_flags_value);
            printf(" %s %-4s ", flags[j].str, (cap_flags_value == CAP_SET) ? "OK" : "NOK");
        }
        printf("\n");
    }
    printf("\n");
}

static void _cap_ambient_dump(cap_value_t i)
{

    int value;
    if (!CAP_AMBIENT_SUPPORTED()) {
        printf("kernel NOT support AMBIENT!\n");
        return;
    }

    value = cap_get_ambient(i);
    if (value < 0) {
        return;
    }
 
    printf(" %s %-4s ", "AMBIENT", (value == CAP_SET) ? "OK" : "NOK");
}


static void cap_ambient_dump(void)
{
    int value;
    cap_value_t i;
 
    if (!CAP_AMBIENT_SUPPORTED()) {
        printf("kernel NOT support AMBIENT!\n");
        return;
    }
 
    /* dump them */
    for (i=0; i < CAP_LAST_CAP; i++) {
        value = cap_get_ambient(i);
        if (value < 0) {
            continue;
        }
 
        printf("%-20s %d\t\t", cap_to_name(i), i);
        printf("flags: \t\t");
        printf(" %s %-4s ", "AMBIENT", (value == CAP_SET) ? "OK" : "NOK");
        printf("\n");
    }
    printf("\n");
}

static void _cap_bound_dump(cap_value_t i)
{
    int value;
    if (!CAP_IS_SUPPORTED(0)) {
        printf("kernel NOT support bound!\n");
        return;
    }

    value = cap_get_bound(i);
    if (value < 0) {
        return;
    }
   
    printf(" %s %-4s ", "BOUND", (value == CAP_SET) ? "OK" : "NOK");
}


static void cap_bound_dump(void)
{
    int value;
    cap_value_t i;
 
    if (!CAP_IS_SUPPORTED(0)) {
        printf("kernel NOT support bound!\n");
        return;
    }
 
    /* dump them */
    for (i=0; i < CAP_LAST_CAP; i++) {
        value = cap_get_bound(i);
        if (value < 0) {
            continue;
        }
 
        printf("%-20s %d\t\t", cap_to_name(i), i);
        printf("flags: \t\t");
        printf(" %s %-4s ", "BOUND", (value == CAP_SET) ? "OK" : "NOK");
        printf("\n");
    }
    printf("\n");
}

void print_all_cap(void);
void print_all_cap(void)
{
    cap_t cap = cap_get_proc();
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
    int j;
    cap_value_t i;
 
    /* dump them */
    for (i=0; i < CAP_LAST_CAP; i++) {
        printf("%-20s %d\t\t", cap_to_name(i), i);
        printf("flags: \t\t");
        for (j=0; j < nitems(flags); j++) {
            cap_get_flag(cap, i, flags[j].flag, &cap_flags_value);
            printf(" %s %-4s ", flags[j].str, (cap_flags_value == CAP_SET) ? "OK" : "NOK");
        }

        _cap_ambient_dump(i);
        _cap_bound_dump(i);
        printf("\n");
    }
    printf("\n");
}

/*
int main()
{
    printf("type 1 : \n");
    print_cap_sys_call(); 
    printf("\n");

#if 0 
    printf("type 2 : \n");
    print_cap_to_text();
    printf("\n");

    printf("type 3 : \n");
    print_cap_get_flag();
    printf("\n");

    printf("ambient : \n");
    cap_ambient_dump();
    printf("\n");


    printf("bound : \n");
    cap_bound_dump();
    printf("\n");
    print_all_cap();
#endif
}

*/