

#include "syscall.h"

int main()
{
  char bufferRead[32];
  int result_open, result_close ,result_write , result_read;
  
  result_open = Open("hello.txt" , 0);
   
  result_write = Write("678", 7 , 1); 
  result_read = Read(bufferRead, 5, result_open);
  
  Halt();
  /* not reached */
}