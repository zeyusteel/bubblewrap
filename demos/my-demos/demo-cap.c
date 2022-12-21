#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#undef _POSIX_SOURCE
#include <sys/capability.h>
#include <unistd.h>
#include <sys/prctl.h>

void print_all_cap(void);

#define lsize(l) (sizeof(l) / sizeof(l[0]))


#define CAP_TO_MASK_0(x) (1L << ((x) & 31))
#define CAP_TO_MASK_1(x) CAP_TO_MASK_0(x - 32)

int cap_set(const int *cap_flag, int cap_flag_size,
                    const cap_value_t *cap_list, int cap_list_size,
                    cap_flag_value_t set);

void cap_mask() 
{
    // 左移是逻辑/算术左移(两者完全相同),左移时总是移位和补零
    // 右移时无符号数是移位和补零，此时称为逻辑右移;而有符号数大多数情况下是移位和补最左边的位（也就是补最高有效位）,此时为算数右移

    printf("index\n");
    printf("%d\n", CAP_TO_INDEX(0));  // 0 右移5位  -> 0
    printf("%d\n", CAP_TO_INDEX(1));  // 1 右移5位  -> 0
    printf("%d\n", CAP_TO_INDEX(32)); // 100000 右移5位 -> 1
    printf("%d\n", CAP_TO_INDEX(33)); // 100001 右移5位 -> 1

    printf("mask\n");
    printf("%d\n", CAP_TO_MASK(0)); // 1 左移(0 & 11111) 0位 -> 1
    printf("%d\n", CAP_TO_MASK(1)); // 1 左移(1 & 11111) 1位 -> 2
    printf("%d\n", CAP_TO_MASK(2)); // 1 左移(2 & 11111) 2位 -> 4

    printf("%u\n", CAP_TO_MASK(31)); // 1 左移(11111 & 11111) 31位 -> 2147483648 ,此时再用%d已经越界了,用%u转换为无符号

    //左移里一个比较特殊的情况是当左移的位数超过该数值类型的最大位数时,编译器会用左移的位数去模类型的最大位数,然后按余数进行移位
    printf("%d\n", CAP_TO_MASK(32)); // 1左移(100000 & 11111) (32 % 31) 1 位 -> 1
}


void cap_get_set()
{
    int rc;
    int flag_list[] = {4};
    cap_value_t cap_list[] = {9};

    int flag_list_new[] = {2};
    rc = cap_set(flag_list_new, lsize(flag_list_new), cap_list, lsize(cap_list), CAP_SET);

    rc = cap_set(flag_list, lsize(flag_list), cap_list, lsize(cap_list), CAP_CLEAR);


    printf("rc: %d\n", rc);

    print_all_cap();
}

void cap_check_exec() 
{
    char path[128] = {'\0'};
    int rc;

    cap_get_set();

    rc = readlink("/proc/self/exe", path, sizeof(path));
    if (rc == -1) {
        printf("readlink %s\n", strerror(errno));
        exit(-1);
    }
    
    rc = execl(path, path, "--test", NULL);
    if (rc < 0) {
        printf("execl %s\n", strerror(errno));
        exit(-1);
    }
}


int main(int argc, char const *argv[])
{
    if (argc == 2 && (strcmp(argv[1], "--test") == 0)) {
        printf("call --test\n");
        print_all_cap();
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--exec") == 0)) {
        cap_check_exec();
        return 0;
    }

    cap_get_set();
    return 0;
}
