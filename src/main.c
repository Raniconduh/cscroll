#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>

#include "dir.h"
#include "io.h"

int main(int argc, char ** argv) {
	curses_init();
	
	if (argc > 1) {
		cwd = malloc(strlen(argv[1]) + 2);
		strcpy(cwd, argv[1]);
	} else {
		char * p = getenv("PWD");
		cwd = malloc(strlen(p) + 2);
		strcpy(cwd, p);
	}

	list_dir(cwd);

	size_t cursor = 1;

	size_t first_f, last_f;
	first_f = cursor - 1;
	last_f = n_dir_entries > ((unsigned)LINES - 6) ? LINES - 6 : n_dir_entries;

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
		
		printw("\n%s\n\n", cwd);

		for (size_t i = first_f; i < last_f; i++) {
			if (cursor - 1 == i)
				curses_write_file(dir_entries[i], true);
			else
				curses_write_file(dir_entries[i], false);
		}

		printw("\n%lu/%lu\n", cursor, n_dir_entries);

		refresh();

		char c = curses_getch();
		if ((c == ARROW_UP || c == 'k') && cursor > 1)
			cursor--;
		else if ((c == ARROW_DOWN || c == 'j') && cursor < n_dir_entries)
			cursor++;
		else if (c == ARROW_LEFT || c == 'h') {
			cd_back();
			free_dir_entries();
			list_dir(cwd);
			cursor = 1;
			first_f = 0;
			last_f = n_dir_entries > ((unsigned)LINES - 6) ? LINES - 6 : n_dir_entries;
		} else if ((c == ARROW_RIGHT || c == 'l') && dir_entries[cursor - 1]->file_type == FILE_DIR) {
			enter_dir(dir_entries[cursor - 1]->name);
			free_dir_entries();
			list_dir(cwd);
			cursor = 1;
			first_f = 0;
			last_f = n_dir_entries > ((unsigned)LINES - 6) ? LINES - 6 : n_dir_entries;
		} else if (c == 'q')
			goto done;
	}

done:
	free_dir_entries();
	free(dir_entries);
	terminate_curses();
	return 0;
}
