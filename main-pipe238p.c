#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

int
main(void)
{
	pid_t c1;
	pid_t c2;
	int pipe1[2], pipe2[2];
	if(pipe(pipe2) < 0){
		perror("Error in pipe1");
	}
	c1 = fork();
	if(c1 == 0){
		//first child
		
		if(pipe(pipe1) < 0){
		perror("Error in pipe2");
		
	}
	c2  = fork();
		if(c2 == 0){
			//second child
			dup2(pipe1[1],1);
			close(pipe1[0]);
			execl("/bin/ls", "ls", NULL);
			exit(0);
		}
		
		//first child
		dup2(pipe1[0], 0);
		close(pipe1[1]);
		dup2(pipe2[1], 1);
		execl("/bin/grep", "grep", "main", NULL);
		
		exit(0);
	}
	else{
		dup2(pipe2[0], 0);
		close(pipe2[1]);
		execlp("/usr/bin/wc", "wc", NULL);
  //parent
  exit(0);
}
}


