#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "copyFiles.h"

int main(int argc, char * argv[])
{
//	printf("You have passed %d arguments\n", argc);
  if (argc == 3){
	//  printf("First argument is %s\n", argv[0]);
	//  printf("%s will be copied to %s\n", argv[1], argv[2]);
	  copyFiles(argv[1], argv[2]);	  	  
  }
  else{
	//  printf("You haven't passed any files as input\n");
	  return 1;
  }
  exit(0);
}
