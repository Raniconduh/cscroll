#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/wait.h>

#include "io.h"
#include "dir.h"
#include "var.h"
#include "opts.h"
#include "main.h"
#include "info.h"
#include "commands.h"


bool cutting = false;
char * cut_start_dir = NULL;
char ** cuts = NULL;


void ext_open(char * file) {
	clear();
	refresh();

	char * f = malloc(strlen(cwd) + strlen(file) + 2);
	sprintf(f, "%s/%s", cwd, file);

	endwin();
	pid_t pid = fork();
	if (!pid) {
		if (!opener.fpath){
#if defined(__APPLE__) || defined(__MACH__)
			execvp("open", (char*[3]){"open", f, NULL});
			refresh();
#else
			execvp("xdg-open", (char*[3]){"xdg-open", f, NULL});
			refresh();
#endif
		} else {
			execvp(opener.fpath, (char*[3]){opener.fpath, f, NULL});
			refresh();
		}
		exit(0);
	}
	wait(NULL);
	free(f);

	initscr();
	clear();
	refresh();
}


long lit_search(long c, char * s) {
	long ret = -1;
	for (long i = c; i < (signed)n_dir_entries; i++) {
		if (!strcmp(s, dir_entries[i]->name)) {
			ret = i;
			break;
		}
	}

	if (ret == -1 && c > 1) {
		for (long i = 0; i < c; i++) {
			if (!strcmp(s, dir_entries[i]->name)) {
				ret = i;
				break;
			}
		}
	}

	return ret;
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
	cut_start_dir = malloc(strlen(wd) + 1);
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
	n_marked_files = 0;
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
	endwin();

	if (!fork()) {
		execvp("sh", (char*[]){"sh", "-c", cmd, NULL});
		exit(0);
	}
	wait(NULL);

	puts("\nPress enter to continue");
	while (fgetc(stdin) != '\n');

	initscr();
	clear();
	refresh();
}


void set(char * v) {
	if (!var_set(v, VAR_TRUE)) {
		display_info(INFO_WARN, "Unknown variable (%s)", v);
	}
}


void unset(char * v) {
	if (!var_set(v, VAR_FALSE)) {
		display_info(INFO_WARN, "Unknown variable (%s)", v);
	}
}


// attempt to "open" the file the cursor is on
void open_cur_file(void) {
	// enter directtory/link pointing to dir
	if (dir_entries[cursor - 1]->file_type == FILE_DIR ||
		dir_entries[cursor - 1]->under_link == FILE_DIR) {
		enter_dir(dir_entries[cursor - 1]->name);
		free_dir_entries();
		list_dir(cwd);
		cursor = 1;
		first_f = 0;
		last_f = LAST_F;
	} else {
		ext_open(dir_entries[cursor - 1]->name);
	}
}
