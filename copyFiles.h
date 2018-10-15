#include <fcntl.h>
#include <unistd.h>

const int READ_SIZE = 40;
void copyFiles(char * sourceFile, char * destFile){
	// printf("Arguments are %s and %s\n", sourceFile, destFile);
	int sourcefd = open(sourceFile, O_RDONLY);
	  if(sourcefd == -1){
		  perror("Error in opening source");
		  exit(1);
	  }
	  int targetfd = open(destFile, O_RDWR | O_CREAT |O_TRUNC , 0777);
	  if(targetfd == -1){
		  perror("Error in opening target");
		  exit(1);
	  }
	  char * buf = malloc(READ_SIZE* sizeof(char));
	  int sizeRead;
	  while(1){
		 sizeRead = read(sourcefd, buf, READ_SIZE);
		 if(sizeRead < 1)
		 break;
	  if(write(targetfd, buf, sizeRead) < 0){
		  perror("write error\n");
		  exit(1);
	  }
  }
  close(sourcefd);
  close(targetfd);
  return;
}
