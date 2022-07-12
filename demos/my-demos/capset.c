#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
 
#undef _POSIX_SOURCE
#include <sys/capability.h>

#define _CAP_EFFECTIVE      0
#define _CAP_PERMITTED      1                       
#define _CAP_INHERITABLE    2
#define _CAP_AMBIENT        3 
#define _CAP_BOUND          4

extern int errno;
  
void whoami(void)
{
    printf("uid=%i  euid=%i  gid=%i\n", getuid(), geteuid(), getgid());
}
 

static int _cap_set(const int *cap_flag, int cap_flag_size,
                    const cap_value_t *cap_list, int cap_list_size,
                    cap_flag_value_t set)
{
    cap_t cap;
    int i;
    int j;
 
    for (i = 0; i < cap_flag_size; ++i)
    {
        if (cap_flag[i] != _CAP_EFFECTIVE
                && cap_flag[i] != _CAP_PERMITTED
                && cap_flag[i] != _CAP_INHERITABLE
                && cap_flag[i] != _CAP_AMBIENT
                && cap_flag[i] != _CAP_BOUND)
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
        if (cap_flag[i] >= _CAP_EFFECTIVE && cap_flag[i] <= _CAP_INHERITABLE) {
            /* set CAP_EFFECTIVE/CAP_PERMITTED/CAP_INHERITABLE cap */
            if (cap_set_flag(cap, cap_flag[i], cap_list_size, cap_list, set) == -1) {
                perror("cap_set_flag:");
                cap_free(cap);
                return -1;
            }
        }  else if (cap_flag[i] == _CAP_BOUND) {
            if (set != CAP_CLEAR)
                continue;
            for (j = 0; j < cap_list_size; ++j) {
                if (cap_drop_bound(cap_list[j]) < 0) {
                    perror("cap_drop_bound fail");
                    return -1;
                }
            }
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

    int am;

    for (i = 0; i < cap_flag_size; ++i)
    {
        if (cap_flag[i] == _CAP_AMBIENT) {
            if (set != CAP_SET)
                continue;
            for (j = 0; j < cap_list_size; ++j) {
                if ((am = cap_set_ambient(cap_list[j], set)) < 0) {
                    perror("cap_set_ambient fail");
                    return -1;
                }
                printf("am:%d\n", am);
            }
        }
    }
 
    return 0;
}

void print_all_cap();
  
int main(int argc, char **argv)
{
    int stat;
    whoami();

    stat = setuid(geteuid());
    pid_t parentPid = getpid();
 
    if(!parentPid)
        return 1;
    
    int capType[] = {_CAP_EFFECTIVE, _CAP_INHERITABLE};
    cap_value_t capList[] = {CAP_SETUID, CAP_SETGID};

    int rc = _cap_set(capType, sizeof(capType) / sizeof(capType[0]), 
                        capList, sizeof(capList) / sizeof(capList[0]), CAP_CLEAR);

    printf("before set:\n");
    print_all_cap();

    int _capType[] = {_CAP_EFFECTIVE, _CAP_INHERITABLE, _CAP_AMBIENT};

    rc = _cap_set(_capType, sizeof(_capType) / sizeof(_capType[0]), 
                        capList, sizeof(capList) / sizeof(capList[0]), CAP_SET);


    printf("%d\n", rc);

    printf("after set:\n");
    print_all_cap();

    return 0;
}
