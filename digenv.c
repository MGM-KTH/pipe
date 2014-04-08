/*
 * Digenv
 *
 * @author: Gustaf Lindstedt
 * @author: Martin Runelöv
 */

#include <stdio.h>
#include <stdlib.h>

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