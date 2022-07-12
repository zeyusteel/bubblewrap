#include <sys/capability.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    unsigned int t = CAP_TO_MASK(32) ;
    printf("%u\n", t);
    return 0;
}
