#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>

#include "dir.h"
#include "io.h"

int main(int argc, char ** argv) {
	curses_init();
	
	if (argc > 1)
		cwd = argv[1];
	else
		cwd = getenv("PWD");

	list_dir(cwd);

	size_t cursor = 1;

	while (true) {
		erase();
		
		printw("\n%s\n\n", cwd);

		for (size_t i = 0; i < n_dir_entries; i++) {
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
		else if (c == 'q')
			goto done;
	}

done:
	free_dir_entries();
	free(dir_entries);
	endwin();
	return 0;
}
