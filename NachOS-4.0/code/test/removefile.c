
#include "syscall.h"

int main()
{
    int result;
    result = Remove("hello.txt");
    Halt();
    /* not reached */
}