#include <stdio.h>
#include <stddef.h>
#include <ncurses.h>

#include "ui.h"
#include "dir.h"

int main(int argc, char ** argv) {
	ui_init();

	const char * cwd = dir_get_cwd();

	dir_t dir;
	dir_list(cwd, &dir);

	size_t cursor = 0;
	ui_set_status("");
	for (;;) {
		ui_erase();

		ui_set_title(cwd);

		ui_print_dir(&dir, cursor, true);
		ui_print_cursor(cursor, dir.len);

		ui_refresh();

		dirent_t * cur_de = &dir.entries[cursor];
		switch (getch()) {
			case KEY_UP:
				if (cursor > 0) cursor--;
				break;
			case KEY_DOWN:
				if (cursor < dir.len - 1) cursor++;
				break;
			case KEY_LEFT:
				if (dir_cd_back(cwd) >= 0) {
					cursor = 0;
					cwd = dir_get_cwd();
					dir_free(&dir);
					dir_list(cwd, &dir);
				}
				break;
			case KEY_RIGHT:
				if (cur_de->type == DE_DIR) {
					if (dir_cd(cwd, cur_de->name) >= 0) {
						cursor = 0;
						cwd = dir_get_cwd();
						dir_free(&dir);
						dir_list(cwd, &dir);
					}
				}
				break;
			case KEY_HOME:
			case 'g':
				cursor = 0;
				break;
			case KEY_END:
			case 'G':
				cursor = dir.len - 1;
				break;
			case 'q':
				goto finished;
			default: break;
		}
	}

finished:
	dir_free(&dir);
	ui_deinit();
}
