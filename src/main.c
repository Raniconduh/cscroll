#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <string.h>

#include "commands.h"
#include "main.h"
#include "dir.h"
#include "io.h"

int main(int argc, char ** argv) {	
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-p")) {
					print_path = true;
			} else {
				cwd = malloc(strlen(argv[i]) + 2);
				strcpy(cwd, argv[i]);
				setenv("PWD", cwd, true);
			}
		}
	}

	if (!cwd) {
		char * p = getenv("PWD");
		cwd = malloc(strlen(p) + 2);
		strcpy(cwd, p);
	}

	curses_init();

	list_dir(cwd);

	size_t cursor = 1;

	size_t first_f, last_f;
	first_f = cursor - 1;
	last_f = LAST_F;

	while (true) {
		erase();

		// scroll down
		if (cursor + 1 >= last_f && last_f < n_dir_entries) {
			first_f++;
			last_f++;
		// scroll back up
		} else if (cursor - 2 <= first_f && first_f > 0) {
			first_f--;
			last_f--;
		}
		
		// print path at top
		printw("\n%s", cwd);
		if (!n_dir_entries) {
			attron(COLOR_PAIR(RED));
			printw("\tPermission Denied");
			attroff(COLOR_PAIR(RED));
		}
		printw("\n\n");

		// print files
		for (size_t i = first_f; i < last_f; i++) {
			if (cursor - 1 == i)
				curses_write_file(dir_entries[i], true);
			else
				curses_write_file(dir_entries[i], false);
		}

		// print cursor / total entries
		if (n_dir_entries)
			printw("\n%lu/%lu\n", cursor, n_dir_entries);

		refresh();

		char c = curses_getch();
		switch (c) {
			case ARROW_UP:
			case 'k':
				if (cursor > 1) cursor--;
				break;
			case ARROW_DOWN:
			case 'j':
				if (cursor < n_dir_entries) cursor++;
				break;
			case ARROW_LEFT:
			case 'h':
				cd_back();
				free_dir_entries();
				list_dir(cwd);
				cursor = 1;
				first_f = 0;
				last_f = LAST_F;
				break;
			case ARROW_RIGHT:
			case 'l':
				if (!n_dir_entries) break;
				if (dir_entries[cursor - 1]->file_type != FILE_DIR) break;
				enter_dir(dir_entries[cursor - 1]->name);
				free_dir_entries();
				list_dir(cwd);
				cursor = 1;
				first_f = 0;
				last_f = LAST_F;
				break;
			case 'g':
				cursor = 1;
				first_f = 0;
				last_f = LAST_F;
				break;
			case 'G':
				cursor = n_dir_entries;
				last_f = n_dir_entries - 1;
				first_f = n_dir_entries > (unsigned)LINES - 6? n_dir_entries - LINES + 5 : -1;
				break;
			case '.':
				show_dot_files = !show_dot_files;
				free_dir_entries();
				list_dir(cwd);
				cursor = 1;
				first_f = 0;
				last_f = LAST_F;
				break;
			case 'd':
				if (dir_entries[cursor - 1]->file_type == FILE_DIR)
					break;
				char * name = dir_entries[cursor - 1]->name;
				char * p = malloc(20 + strlen(name));
				char * args[] = {"No", "Yes", NULL};
				sprintf(p, "Delete the file '%s'?", name);
				char * resp = prompt(p, args);
				free(p);
				if (!strcmp(resp, "Yes")) {
					p = malloc(strlen(cwd) + strlen(name) + 3);
					sprintf(p, "%s/%s", cwd, name);
					remove(p);
					free(p);
					free_dir_entries();
					list_dir(cwd);

					cursor = 1;
					first_f = 0;
					last_f = LAST_F;
				}
				break;
			case 'o':
				ext_open(dir_entries[cursor - 1]->name);
				break;
			case 'q':
				goto done;
			default:
				break;
		}
	}

done:
	free_dir_entries();
	free(dir_entries);
	terminate_curses();

	if (print_path) puts(cwd);

	return 0;
}
