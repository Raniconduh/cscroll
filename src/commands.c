#include <stdlib.h>
#include <unistd.h>

#include "dir.h"
#include "commands.h"

void ext_open(char * file) {
	pid_t pid = fork();
	if (!pid) {
		execvp("xdg-open", (char*[3]){"xdg-open", file, NULL});
		exit(0);
	}
}

