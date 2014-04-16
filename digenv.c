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
void create_child(int argc, char **argv, void (*commands[]) (int nargc, char **nargv), int cmd_counter);
void printenv(int argc, char **argv);
void grep(int argc, char **argv);
void sort(int argc, char **argv);
void pager(int argc, char **argv);

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
	 * An array of function pointers (void*)
	 */
	int cmds = 3;
	void (*commands[cmds]) (int nargc, char**nargv);
	commands[0] = printenv;
	commands[1] = grep;
	commands[2] = sort;
	//commands[3] = less;


	/*
	retval = pipe(pipe_desc);
	if(-1 == retval) {
		fprintf(stderr, "error opening pipe\n");
		exit(1);
	}
	*/
	/*
	 * Special pipe since it is from the root parent, we want the child to output to STDOUT
	 */
	/*pipe_desc[WRITE] = STDOUT;*/

	/*
	 * Pipe is closed within create_child
	 */
	create_child(argc, argv, commands, cmds);

	//pipe_through();
	pager(argc, argv);

	return 0;
}

/*
 * Functions are executed in reverse order, starting from cmd_counter until it hits 0
 */
void create_child(int argc, char **argv, void (*commands[]) (int nargc, char **nargv), int cmd_counter) {
	pid_t child_pid;
	int retval;
	int pipe_desc[2];

	retval = pipe(pipe_desc);
	if(-1 == retval) {
		perror("error opening pipe");
		exit(1);
	}

	child_pid = fork();

	if(0 == child_pid) {

		fprintf(stderr, "child reporting in for duty with pid %d\n", getpid());

		/*
 		 * Child wants to write to the pipe
 		 */
		apply_pipe(pipe_desc, WRITE, STDOUT);

		close_pipe(pipe_desc);

		fprintf(stderr, "child closed pipe\n");

		fprintf(stderr, "child executing...\n");

		/*
 		 * If there are more commands left, create a pipe and spawn a child
 		 */
		if(--cmd_counter > 0) {
			create_child(argc, argv, commands, cmd_counter);
		}

		/*
 		 * Execute command
 		 */
		commands[cmd_counter] (argc, argv);

		exit(0);
		
	}else{
		int status;

		if(-1 == child_pid) {
			perror("fork() failed!");
			exit(1);
		}

		/*
 		 * Parent wants to read from the pipe
 		 */
		apply_pipe(pipe_desc, READ, STDIN);

		close_pipe(pipe_desc);

		/*
 		 * Wait for child to close
 		 */
		waitpid(child_pid, &status, 0); // 0: No options

		return;
	}
}

void printenv(int argc, char **argv) {
	//sleep(1);
	/*
	int retval;
	int pipe_desc[2];

	retval = pipe(pipe_desc);
	if(-1 == retval) {
		fprintf(stderr, "error opening pipe\n");
		exit(1);
	}

	apply_pipe(pipe_desc, READ, STDOUT);

	//create_child(argc, argv, pipe_desc, (void*)less);
	*/
	char *nargv[5] = {"printenv", NULL};
	execvp("printenv", nargv);
	perror("failed to execute printenv");
}

/*
 * Grep will return:
 *  0     One or more lines were selected.
 *  1     No lines were selected.
 *  >1    An error occurred.
 */
void grep(int argc, char **argv) {
	if(argc > 1) {
		//sleep(1);
		/* Do stuff, like parse argv */
		/* Temp */
		//pipe_through();
		argv[0] = "grep"; // first argument is command name
		execvp("grep", argv);
		perror("failed to execute grep");
	}
	else {
		// execvp("cat", argv);
		pipe_through();
	}
	return;
}

void sort(int argc, char **argv) {
	char *nargv[5] = {"sort", NULL};
	execvp("sort", nargv);
	perror("failed to execute sort");
}

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
		perror("error closing pipe");
		exit(1);
	}
	retval = close(pipe[WRITE]);
	if(-1 == retval) {
		perror("error closing");
		exit(1);
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
			exit(1);
		}
	}
	// size_t bytes;
	// while(1) {
	// 	bytes = fread(buffer, sizeof(char), 512, stdin);
	// 	fwrite(buffer, sizeof(char), 512, stdout);
	// 	fflush(stdout);
	// 	if(bytes<512 && feof(stdin))
	// 		break;
	// }
}
