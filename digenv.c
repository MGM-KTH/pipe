/*
 * NAME
 *     Digenv - Digs (searches) for environment variables and prints a sorted list of the results
 *
 * SYNTAX
 *     digenv [-abcdDEFGHhIiJLlmnOopqRSsUVvwxZ] [-A num] [-B num] [-C[num]]
          [-e pattern] [-f file] [--binary-files=value] [--color[=when]]
          [--colour[=when]] [--context[=num]] [--label] [--line-buffered]
          [--null] [pattern]
 * DESCRIPTION
 *     Without parameters the program corresponds to "printenv | sort | PAGER".
 *     Otherwise, it corresponds to "printenv | grep [args] | sort | PAGER".
 *
 * OPTIONS
 *     see grep(1)
 *
 * EXAMPLES 
 *      digenv
 *      digenv PAGER
 *      digenv -c SH*
 *
 * ENVIRONMENT
 *     PAGER, sets which pager is used to display the result. Defaults to 'less'
 *
 * SEE ALSO
 *     printenv(1), grep(1), sort(1), less(1)
 *
 * DIAGNOSTICS 
 *     0: The program exited normally
 *     1: The program encountered an error, or no lines were selected
 *     >1: The program encountered an error
 * NOTES
 *     If no matches are found the pager is not used.
 *
 * AUTHORS
 *      Gustaf Lindstedt (glindste@kth.se)
 *      Martin Runelöv (mrunelov@kth.se)
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

 #ifdef __APPLE__
#include <sys/wait.h>
#else
#include <wait.h>
#endif

#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define LINELENGTH 256 // used by stdin-->stdout (pipe_through/cat)

void apply_pipe(int pipe[2], int pfd, int fd);
void close_pipe(int pipe[2]);
void pipe_through();
void register_sighandler(int signal_code, void (*handler) (int sig));
int create_child(
	int argc, 
	char **argv, 
	void (*commands[]) (int nargc, char **nargv),
	int cmd_counter
);
void printenv(int argc, char **argv);
void grep(int argc, char **argv);
void sort(int argc, char **argv);
void pager(int argc, char **argv);

/* register_sighandler
 *
 * Registers a signal handler
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
		exit(EXIT_FAILURE);
	}
}

/* main
 *
 */
int main(int argc, char **argv, char **envp) {
	/*
	 * An array of function pointers (void*)
	 */
	int cmds = 3;
	void (*commands[cmds]) (int nargc, char**nargv);
	commands[0] = printenv;
	commands[1] = grep;
	commands[2] = sort;

	/*
	 * Pipe is closed within create_child
	 */
	int status = create_child(argc, argv, commands, cmds);
	if(status) {
		exit(status);
	}

	pager(argc, argv);

	return 0;
}

/* create_child
 *
 * create_child returns the exit status of the child process it creates.
 *
 * Creates child processes that executes the functions specified in 'commands'.
 * cmd_counter is the number of functions to be executed.
 * Functions are executed in reverse order.
 */
int create_child(int argc, char **argv, void (*commands[]) (int nargc, char **nargv), int cmd_counter) {
	pid_t child_pid;
	int retval;
	int pipe_desc[2];
	int status;

	retval = pipe(pipe_desc);
	if(-1 == retval) {
		perror("error opening pipe");
		exit(EXIT_FAILURE);
	}

	child_pid = fork();

	if(0 == child_pid) {
		/*
 		 * Child writes to the pipe
 		 */
		apply_pipe(pipe_desc, WRITE, STDOUT);

		close_pipe(pipe_desc);

		/*
 		 * If there are more commands left, create a pipe and spawn a child
 		 */
		if(--cmd_counter > 0) {
			status = create_child(argc, argv, commands, cmd_counter);
			if (status) {
				exit(status);
			}
		}

		/*
 		 * Execute command
 		 */
		commands[cmd_counter] (argc, argv);

		exit(EXIT_SUCCESS);

	}else{
		if(-1 == child_pid) {
			perror("fork() failed!");
			exit(EXIT_FAILURE);
		}

		/*
 		 * Parent reads from the pipe
 		 */
		apply_pipe(pipe_desc, READ, STDIN);
		close_pipe(pipe_desc);

		/*
 		 * Wait for child to close
 		 */
		waitpid(child_pid, &status, 0); /* 0: No options */
		return WEXITSTATUS(status);
	}
}

/*
 * Runs printenv
 */
void printenv(int argc, char **argv) {
	char *nargv[5] = {"printenv", NULL};
	execvp("printenv", nargv);
	perror("failed to execute printenv");
}

/*
 * Runs grep
 * grep will return:
 *  0     One or more lines were selected.
 *  1     No lines were selected.
 *  >1    An error occurred.
 */
void grep(int argc, char **argv) {
	if(argc > 1) {
		/* argv[0] = "grep";  first argument is command name. Currently set to "digenv". */
		execvp("grep", argv);
		perror("failed to execute grep");
	}
	else {
		pipe_through();
	}
	return;
}

/*
 * Runs sort
 */
void sort(int argc, char **argv) {
	char *nargv[5] = {"sort", NULL};
	execvp("sort", nargv);
	perror("failed to execute sort");
}

/*
 * Prints from stdin to stdout using the pager defined in the
 * environment variable PAGER. Defaults to less.
 */
void pager(int argc, char **argv) {
	char* pager = getenv("PAGER");
	if(!pager) {
		pager = "less";
	}
	char *nargv[5] = {pager, NULL};
	execvp(pager, nargv);
	perror("failed to execute less");
}

/*
 * Apply the pipe file descriptors to STDIN and STDOUT
 */
void apply_pipe(int pipe[2], int pfd, int fd) {
	int retval;
	retval = dup2(pipe[pfd], fd);
	if(-1 == retval) {
		perror("error copying file descriptor to stdin");
		exit(EXIT_FAILURE);
	}
}

/*
 * Close the file descriptors in the pipe
 */
void close_pipe(int pipe[2]) {
	int retval;
	retval = close(pipe[READ]);
	if(-1 == retval) {
		perror("error closing pipe");
		exit(EXIT_FAILURE);
	}
	retval = close(pipe[WRITE]);
	if(-1 == retval) {
		perror("error closing");
		exit(EXIT_FAILURE);
	}
}

/*
 * Emulates 'cat' by throughputting stdin to stdout
 */
void pipe_through() {
	char line[512];
	while(fgets(line,LINELENGTH,stdin)) {
		if(fputs(line,stdout)==EOF) {
			perror("Write to stdout failed");
			exit(EXIT_FAILURE);
		}
	}
}
