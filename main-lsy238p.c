#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
int main(void){
	
	/*pid_t pid = fork();
	if(pid < 0){
		perror("Fork error");
		exit(1);
	}
	if (pid == 0){ */
		int ydesc = open("y", O_RDWR | O_CREAT |O_TRUNC , 0777);
	if(ydesc == -1){
		perror("Error in opening y file");
		exit(1);
	}
	dup2(ydesc, 1);
	char* loc[2];
	loc[0] = "ls";
	loc[1] = NULL;
	execv("/bin/ls", loc);
	// }
}
