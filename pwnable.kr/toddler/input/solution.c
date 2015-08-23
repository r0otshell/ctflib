/* File:        solution.c
 * Author:      solstice
 * Description: Solution to the 'input' challenge on pwnable.kr
 */

#define TARGET      "/home/input/input"
#define BIND_PORT   "8779"
#define SERVER_ADDR "127.0.0.1"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

void error_handler(char*, int);
void target_proc_ctl(char**,char**,int*,int*);
void socket_to_target();
void run_parent(int*, int*);

int main() {

	// set up argv stage 1
	char *argv[101] = {
		[0 ... 99 ] = "A"
	};
	argv['A'] = "\x00";
	argv['B'] = "\x20\x0a\x0d";

	// set up stdin pipe stage 2
	int stdin_pipe[2];
	if ( pipe(stdin_pipe) == -1 ) {
		
		error_handler("Unable to create stdin pipe", errno);
	}

	// set up stdout pipe stage 2
	int stderr_pipe[2];
	if ( pipe(stderr_pipe) == -1 ) {
		
		error_handler("Unable to create stdin pipe", errno);
	}

	// set up envp for stage 3
	char *envp[2] = {
		"\xde\xad\xbe\xef=\xca\xfe\xba\xbe",	
		0x00
	};

	// create file for stage 4
	FILE *fp = fopen("\x0a", "w");
	if ( fp == NULL ) {

		error_handler("Unable to open file in write mode.\n", errno);
	}
	if ( fwrite("\x00\x00\x00\x00", 4, 1, fp) == 0) {

		error_handler("Unable to write to file pointer.\n", -1);
	}
	fclose(fp);

	// set bind port for stage 5
	argv['C'] = BIND_PORT;

	pid_t cpid = fork();
	switch(cpid) {
	case -1:
		error_handler("Unable to fork.\n", -1);
	case 0:
		target_proc_ctl(argv, envp, stdin_pipe, stderr_pipe);
	default:
		run_parent(stdin_pipe, stderr_pipe);
	}

	return 0;
}

void error_handler(char* msg, int error_code) {

	printf(msg);	

	if (error_code != -1) {
		
		fprintf(stdout, "Errno: %d\n", error_code);
	}

	exit(1);
}

void target_proc_ctl(char **argv,char **envp,int *stdin_pipe,int *stderr_pipe){

	// close the write file desriptors of the stdin_pipe and stderr_pipe 
	close(stdin_pipe[1]);
	close(stderr_pipe[1]);

	// overwrite child process stdin with read file descriptor of stdin_pipe
	if ( dup2(stdin_pipe[0], 0) == -1 ) {

		error_handler("stdin dup2 failed\n", errno);
	}

	// overwrite child process stderr with read file descriptor of stderr_pipe
	if ( dup2(stderr_pipe[0], 2) == -1) {

		error_handler("stderr dup2 failed\n", errno);
	}
	
	if (execve(TARGET, argv, envp) == -1) {
	
		error_handler("Execve failed\n", errno);
	}
}

void socket_to_target() {

	// create a socket for stage 5
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket < 0) {

		error_handler("Unable to create socket\n", errno);	
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(BIND_PORT));
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	send(sockfd, "\xde\xad\xbe\xef", 4, 0);
	close(sockfd);
}
				
void run_parent(int *stdin_pipe, int *stderr_pipe) {

	// wait for child process because it's hella slow
	sleep(1);

	// close read file descriptors of stdin_pipe and stderr_pipe
	close(stdin_pipe[0]);
	close(stderr_pipe[0]);

	// pipe \x00\x0a\x00\xff to stdin of target process
	if ( write(stdin_pipe[1], "\x00\x0a\x00\xff", 4) == -1) {

		error_handler("unable to write to stdin_pipe\n", errno);
	}

	// close write file descriptor of stdin_pipe to indicate EOF
	close(stdin_pipe[1]);

	// pipe \x00\x0a\x02\xff stderr of target process
	if ( write(stderr_pipe[1], "\x00\x0a\x02\xff", 4) == -1) {

		error_handler("unable to write to stderr_pipe\n", errno);
	}

	// close write file descriptor of stderr_pipe to indicate EOF
	close(stderr_pipe[1]);

	// ... wait for child process some more >_>
	sleep(6);

	socket_to_target();

	wait(NULL);

	// remove the weird file created for stage 4
	unlink("0x0a");
}
