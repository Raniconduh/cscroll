#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/wait.h>

#include "dir.h"
#include "commands.h"

void ext_open(char * file) {
	char * f = malloc(strlen(cwd) + strlen(file) + 2);
	sprintf(f, "%s/%s", cwd, file);

	pid_t pid = fork();
	if (!pid) {
		execvp("xdg-open", (char*[3]){"xdg-open", f, NULL});
		exit(0);
	}
	wait(NULL);
	free(f);

	erase();
	refresh();
}


long search_file(long c, char * s) {
	long ret = -1;
	regex_t r;
	regcomp(&r, s, REG_EXTENDED);
	for (long i = c; i < (signed)n_dir_entries; i++) {
		if (!regexec(&r, dir_entries[i]->name, 0, NULL, 0)) {
			ret = i;
			break;
		}
	}

	if (ret == -1 && c > 1) {
		for (long i = 0; i < c; i++)
			if (!regexec(&r, dir_entries[i]->name, 0, NULL, 0)) {
				ret = i;
				break;
			}
	}
	regfree(&r);
	return ret;
}

