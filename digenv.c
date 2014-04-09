/*
 * Digenv
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

    /* no parameters corresponds to "printenv | sort | less".
     * the pager used (default: less) is set by the env. var. PAGER.
     * We can read env. vars. with "getenv(const char* name)"
     */

	int i;
	for (i = 0; envp[i] != NULL; ++i) {
		printf("%2d:%s\n", i, envp[i]);
	}
	return 0;
}
