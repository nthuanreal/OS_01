#include "syscall.h"

int main(int argc, char* argv[])
{
  int result = -1;
  char* filename ="hello_world.txt" ;
  if(argc==1){
      result = Create(argv);
  }
  else{
    result = Create(filename);
  }

  Halt();
  return result;
  /* not reached */
}


