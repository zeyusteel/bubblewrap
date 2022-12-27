
#include <sys/prctl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#undef _POSIX_SOURCE
#include <sys/capability.h>

#define lsize(l) (sizeof(l) / sizeof(l[0]))

// https://lwn.net/Articles/478062/
//#define PR_SET_NO_NEW_PRIVS 36
//#define PR_GET_NO_NEW_PRIVS 37

//设置之后,当前进程以及子进程将无法添加权限，但是可以去除权限

void print_all_cap(void);

int cap_set(const int *cap_flag, int cap_flag_size,
                    const cap_value_t *cap_list, int cap_list_size,
                    cap_flag_value_t set);



int test_cap_set(int cap)
{
    int rc;
    int flag_list[] = {0, 1, 2, 3};
    cap_value_t cap_list[] = {cap};


    rc = cap_set(flag_list, lsize(flag_list), cap_list, lsize(cap_list), CAP_SET);

    print_all_cap();
    return rc;
}

int test_no_new_privs()
{
  int nnp = prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0);
  if (nnp == -EINVAL) {
    printf("Failed!\n");
    return -1;
  }

  printf("nnp was %d\n", nnp);

#if 1
  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
    printf("Failed!\n");
    return -1;
  }

  nnp = prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0);
  if (nnp == -EINVAL) {
    printf("Failed!\n");
    return -1;
  }

  printf("nnp is %d\n", nnp);

#endif

  printf("here goes...\n");
  execlp("bash", "bash", NULL); //sudo将没有权限
  printf("Failed to exec bash\n");

  return 0;
}

int main(int argc, char const *argv[])
{
    test_no_new_privs();
    return 0;
}