#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/wait.h>

#include "io.h"
#include "dir.h"
#include "opts.h"
#include "commands.h"


bool cutting = false;
char * cut_start_dir = NULL;
char ** cuts = NULL;


void ext_open(char * file) {
	clear();
	refresh();

	char * f = malloc(strlen(cwd) + strlen(file) + 2);
	sprintf(f, "%s/%s", cwd, file);

	pid_t pid = fork();
	if (!pid) {
		execvp("xdg-open", (char*[3]){"xdg-open", f, NULL});
		exit(0);
	}
	wait(NULL);
	free(f);

	clear();
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


void create_cuts(char * wd, char ** ls) {
	cut_start_dir = malloc(strlen(wd));
	strcpy(cut_start_dir, wd);

	cutting = true;
	if (!cuts) cuts = malloc(0);
	// if not null, use passed list
	if (ls) {
		size_t total_cuts = 0;
		for (char ** p = ls; *p; p++) total_cuts++;
		cuts = realloc(cuts, sizeof(char*) * (total_cuts + 1));
		size_t i;
		for (i = 0; i < total_cuts; i++) {
			cuts[i] = malloc(strlen(ls[i]) + 1);
			strcpy(cuts[i], ls[i]);
		}
		cuts[i] = NULL;
		return;
	}
	// else use marked files
	// count total number of cuts to make
	size_t total_cuts = 0;
	for (size_t i = 0; i < n_dir_entries; i++)
		if (dir_entries[i]->marked) total_cuts++;
	cuts = realloc(cuts, sizeof(char*) * (total_cuts + 1));
	// store cuts into list
	char ** p = cuts;
	for (size_t i = 0; i < n_dir_entries; i++) {
		if (dir_entries[i]->marked) {
			*p = malloc(strlen(dir_entries[i]->name) + 1);
			strcpy(*p, dir_entries[i]->name);
			p++;
		}
	}
	*p = NULL;
}


void free_cuts(void) {
	cutting = false;
	for (char ** p = cuts; *p; p++)
		free(*p);
	free(cuts);
	cuts = NULL;

	free(cut_start_dir);
	cut_start_dir = NULL;
}


void paste_cuts(char * path) {
	cutting = false;
	for (char ** p = cuts; *p; p++) {
		char old_path[strlen(cut_start_dir) + strlen(*p) + 1];
		char new_path[strlen(path) + strlen(*p) + 1];
		sprintf(old_path, "%s/%s", cut_start_dir, *p);
		sprintf(new_path, "%s/%s", path, *p);
		rename(old_path, new_path);
	}
}


void run_cmd(char * cmd) {
	clear();
	refresh();

	if (!fork()) {
		execvp("sh", (char*[]){"sh", "-c", cmd, NULL});
		exit(0);
	}
	wait(NULL);

	addstr("\nPress enter to continue\n");
	refresh();
	while (getch() != '\n');

	clear();
	refresh();
}


void set(char * v) {
	if (!strcmp(v, "color")) {
		color = true;
		set_color();
	}
#if ICONS
	else if (!strcmp(v, "icons")) show_icons = true;
#endif
	else {
		printw("Unknown variable (%s)", v);
		refresh();
		napms(500);
	}
}


void unset(char * v) {
	if (!strcmp(v, "color")) {
		color = false;
		set_color();
	}
#if ICONS
	else if (!strcmp(v, "icons")) show_icons = false;
#endif
	else {
		printw("Unknown variable (%s)", v);
		refresh();
		napms(500);
	}
}
