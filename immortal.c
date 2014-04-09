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

void kill_handler(int signal_code) {
    /* Do nothing */
    printf("%s\n", "kill_handler called");
}

int main() {
    register_sighandler(SIGINT, kill_handler);
    int a = 0;
    while(1) {
        ++a;
    }
}