#include <sys/prctl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

// https://lwn.net/Articles/478062/
//#define PR_SET_NO_NEW_PRIVS 36
//#define PR_GET_NO_NEW_PRIVS 37

//设置之后,当前进程以及子进程将无法添加权限，但是可以去除权限

int main()
{
  int nnp = prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0);
  if (nnp == -EINVAL) {
    printf("Failed!\n");
    return 1;
  }

  printf("nnp was %d\n", nnp);

  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
    printf("Failed!\n");
    return 1;
  }

  nnp = prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0);
  if (nnp == -EINVAL) {
    printf("Failed!\n");
    return 1;
  }

  printf("nnp is %d\n", nnp);

  printf("here goes...\n");
  execlp("bash", "bash", NULL);
  printf("Failed to exec bash\n");
  return 1;
}
