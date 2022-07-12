#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>

int main(int argc, char const *argv[])
{
    int rc = prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    printf("rc=%d\n", rc);
    return 0;
}
