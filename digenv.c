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

void close_pipe(int pipe[2]);

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

	/* 
	 * We can read env. vars. with "getenv(const char* name)" 
	 * e.g. getenv("PAGER"). If 0, use less (?)
	 * execvp("grep",argv) ? something like that...
	 */

	/*
	 * Grep will return:
	 *
	 *  0     One or more lines were selected.
	 *  1     No lines were selected.
	 *  >1    An error occurred.
	 *
	 */

	/*int i;*/
	int retval;
	int pipe_desc[2];
	pid_t child_pid;

	/*
	for (i = 0; envp[i] != NULL; ++i) {
		printf("%2d:%s\n", i, envp[i]);
	}
	*/

	retval = pipe(pipe_desc);

	child_pid = fork();

	if(0 == child_pid) {
		/*char buf[512];*/

		fprintf(stderr, "child reporting in for duty with pid %d\n", getpid());

		/*
 		 * Overwrite 
 		 */
		retval = dup2(pipe_desc[0], 0);
		if(-1 == retval) {
			fprintf(stderr, "error copying file descriptor to stdin\n");
			exit(1);
		}
		fprintf(stderr, "child duplicated pipe\n");

		close_pipe(pipe_desc);

		fprintf(stderr, "child closed pipe\n");

		/*
		retval = read(0, buf, sizeof(buf)/sizeof(char));
		if(-1 == retval) {
			fprintf(stderr, "error reading from stdin\n");
			exit(1);
		}
		*/

		fprintf(stderr, "child executing printenv...\n");
		
		execvp("printenv", argv);

		/*
		retval = write(1, buf, retval);
		if(-1 == retval) {
			fprintf(stderr, "error writing to stdout\n");
			exit(1);
		}
		*/
		exit(0);
		
	}else{
		int status;

		if(-1 == child_pid) {
			perror("fork() failed!\n");
			exit(1);
		}

		/*
		retval = dup2(pipe_desc[1], 1);
		if(-1 == retval) {
			fprintf(stderr, "error copying file descriptor to stdout\n");
			exit(1);
		}
		*/

		close_pipe(pipe_desc);

		waitpid(child_pid, &status, 0);

		exit(0);
	}

	return 0;
}

/*
 * Close the file descriptors in the pipe
 */
void close_pipe(int pipe[2]) {
	int retval;
	retval = close(pipe[0]);
	if(-1 == retval) {
		fprintf(stderr, "error closing pipe\n");
		exit(1);
	}
	retval = close(pipe[1]);
	if(-1 == retval) {
		fprintf(stderr, "error closing\n");
		exit(1);
	}
}
