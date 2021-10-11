#include <stdlib.h>
#include <unistd.h>

#include "dir.h"
#include "commands.h"

void ext_open(char * file) {
	char * f = malloc(strlen(cwd) + strlen(f) + 2);
	sprintf(f, "%s/%s", cwd, file);

	pid_t pid = fork();
	if (!pid) {
		execvp("xdg-open", (char*[3]){"xdg-open", f, NULL});
		exit(0);
	}

	free(f);
}

