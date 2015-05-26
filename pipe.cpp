#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
	if(argc != 3) {
		puts("Command Error.");
		exit(-1);
	}

	int myp[2];
	pid_t mypid;

	if(pipe(myp) == -1) {
		puts("Getting Pipe Error: ");
		exit(-1);
	}

	if( (mypid = fork()) == -1 ) {
		perror("Pipe Fork Error: ");
		exit(-1);
	}

	if( mypid > 0) {
		close(myp[1]); //close parent

		if(dup2(myp[0], 0) == -1) {
			perror("Could not write into pipe now: ");
			exit(-1);
		}
		close(myp[0]);

		char * mycmd[] = {argv[2], NULL};
		if(execv(argv[2], mycmd) == -1) {
			perror("Exec Error:");
			exit(-1);
		}
	}

	close(myp[0]);
	if(dup2(myp[1], 1) == -1) {
		perror("Could not read from pipe now: ");
		exit(-1);
	}
	close(myp[1]);

	char *mycmd[] = {argv[1], NULL};
	if(execv(argv[1], mycmd) == -1) {
		perror("Exec Error:");
		exit(-1);
	}
	while(wait(0)!=-1);
}

