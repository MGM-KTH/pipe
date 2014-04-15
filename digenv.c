/*
 * NAME
 *     Digenv - Dig (search) for environment variables
 *
 * DESCRIPTION
 *     Without parameters the program corresponds to "printenv | sort | less".
 *     the pager used (default: less) is set by the environment variable PAGER.
 *
 * SEE ALSO
 *     printenv(1), grep(1), sort(1), less(1)
 *
 * @author: Gustaf Lindstedt
 * @author: Martin Runelöv
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1
#define STDERR 2

void apply_pipe(int pipe[2], int pfd, int fd);
void close_pipe(int pipe[2]);
void register_sighandler(int signal_code, void (*handler) (int sig));
void create_child(int argc, char **argv, int pipe_desc[2], void (*next_command) (int nargc, char **nargv));
void printenv();
void grep();
void sort();
void less();

/*
 * Register a signal handler
 */
void register_sighandler( int signal_code, void (*handler) (int sig) )  {
	int retval;

	struct sigaction signal_parameters;

	signal_parameters.sa_handler = handler;
	sigemptyset(&signal_parameters.sa_mask);
	signal_parameters.sa_flags = 0;

	retval = sigaction(signal_code, &signal_parameters, (void *) 0);

	if(-1 == retval) {
		perror("sigaction() failed");
		exit(1);
	}
}

int main(int argc, char **argv, char **envp) {
	/*int i;*/
	/*
	for (i = 0; envp[i] != NULL; ++i) {
		printf("%2d:%s\n", i, envp[i]);
	}
	*/

	int retval;
	int pipe_desc[2];


	retval = pipe(pipe_desc);
	if(-1 == retval) {
		fprintf(stderr, "error opening pipe\n");
		exit(1);
	}

	create_child(argc, argv, pipe_desc, (void*)printenv);

	return 0;
}

void create_child(int argc, char **argv, int pipe_desc[2], void (*next_command) (int nargc, char **nargv)) {
	pid_t child_pid;

	child_pid = fork();

	if(0 == child_pid) {

		fprintf(stderr, "child reporting in for duty with pid %d\n", getpid());

		apply_pipe(pipe_desc, WRITE, STDIN);

		close_pipe(pipe_desc);

		fprintf(stderr, "child closed pipe\n");

		fprintf(stderr, "child executing...\n");

		next_command(argc, argv);

		exit(0);
		
	}else{
		int status;

		if(-1 == child_pid) {
			perror("fork() failed!\n");
			exit(1);
		}

		close_pipe(pipe_desc);

		waitpid(child_pid, &status, 0); // 0: No options

		exit(0);
	}
}

void printenv(int argc, char **argv) {
	perror("here");
	execvp("printenv", argv);
}

/*
 * Grep will return:
 *  0     One or more lines were selected.
 *  1     No lines were selected.
 *  >1    An error occurred.
 */
void grep() {
}

void sort() {
}

void less() {
}

/*
 * Apply the pipe file descriptors to STDIN and STDOUT
 */
void apply_pipe(int pipe[2], int pfd, int fd) {
	int retval;
	retval = dup2(pipe[pfd], fd);
	if(-1 == retval) {
		fprintf(stderr, "error copying file descriptor to stdin\n");
		exit(1);
	}
}

/*
 * Close the file descriptors in the pipe
 */
void close_pipe(int pipe[2]) {
	int retval;
	retval = close(pipe[READ]);
	if(-1 == retval) {
		fprintf(stderr, "error closing pipe\n");
		exit(1);
	}
	retval = close(pipe[WRITE]);
	if(-1 == retval) {
		fprintf(stderr, "error closing\n");
		exit(1);
	}
}
