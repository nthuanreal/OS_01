

#include "syscall.h"

int main()
{
  int result;
  
  result = Create("hello.txt");
  Halt();
  /* not reached */
} 