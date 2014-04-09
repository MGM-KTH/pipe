/*
 * 
 * Digenv - Dig (search) for environment variables
 *
 * Without parameters the program corresponds to "printenv | sort | less".
 * the pager used (default: less) is set by the environment variable PAGER.
 * 
 *
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

	int i;
	for (i = 0; envp[i] != NULL; ++i) {
		printf("%2d:%s\n", i, envp[i]);
	}
	return 0;
}
