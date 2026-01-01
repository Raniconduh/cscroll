#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <ncurses.h>

#include "ui.h"
#include "dir.h"
#include "config.h"
#include "cvector.h"

int main(int argc, char ** argv) {
	const char * cwd = dir_get_cwd();

	dir_t dir;
	dir_list(cwd, &dir);
	dir_sort(&dir);

	ui_init();
	size_t cursor = 0;
	ui_status_info("");
	for (;;) {
		ui_erase();

		size_t dirlen = dir_len(&dir);
		cvector(dirent_t) entries = dir_entries(&dir);

		ui_set_title(cwd);
		ui_print_dir(&dir, cursor);
		ui_print_cursor(cursor, dirlen);

		ui_refresh();

		dirent_t * cur_de;
		if (dirlen == 0) cur_de = NULL;
		else cur_de = &entries[cursor];

		switch (getch()) {
			case KEY_UP:
				if (cursor > 0) cursor--;
				ui_status_info("");
				break;
			case KEY_DOWN:
				if (cursor < dirlen - 1) cursor++;
				ui_status_info("");
				break;
			case KEY_LEFT: {
				// the string from dir_basename(cwd) may be a static field
				// so it might not be safe to use it after changing cwd
				const char * truebasename = dir_basename(cwd);
				char * basename = NULL;
				if (truebasename) basename = strdup(truebasename);
				int ret = dir_cd_back(cwd);
				if (ret >= 0) {
					cwd = dir_get_cwd();
					dir_free(&dir);
					ret = dir_list(cwd, &dir);
					if (ret < 0) {
						ui_status_error(strerror(-ret));
					} else {
						dir_sort(&dir);
						if (basename) {
							size_t idx;
							int i = dir_search_name(&dir, basename, &idx);
							if (i >= 0) cursor = idx;
						}
						ui_status_info("");
					}
				} else {
					ui_status_error(strerror(-ret));
				}
				free(basename);
				break;
			}
			case KEY_RIGHT:
				if (cur_de && (cur_de->type == DE_DIR || (cur_de->type == DE_LINK
				  && cur_de->linktype == DE_DIR))) {
					int ret = dir_cd(cwd, cur_de->name);
					if (ret >= 0) {
						cursor = 0;
						cwd = dir_get_cwd();
						dir_free(&dir);
						dir_list(cwd, &dir);
						dir_sort(&dir);
						if (dirlen == 0) ui_status_info("Empty Directory");
					} else {
						ui_status_error(strerror(-ret));
					}
				} else {
					ui_status_info("");
				}
				break;
			case KEY_HOME:
			case 'g':
				cursor = 0;
				ui_status_info("");
				break;
			case KEY_END:
			case 'G':
				cursor = dirlen - 1;
				ui_status_info("");
				break;
			case '.': {
				config.dots = !config.dots;
				size_t idx;
				int ret = dir_search_name(&dir, cur_de->name, &idx);
				if (ret < 0) cursor = 0;
				else cursor = idx;
				break;
			}
			case 'l':
				// cycle through long mode options
				if (config.longmode) {
					if (config.longinline) config.longinline = false;
					else config.longmode = false;
				} else {
					config.longmode = true;
					config.longinline = true;
				}
				break;
			case '/': {
				const char * input = ui_readline("/");
				if (!input) break;
				size_t idx;
				int i = dir_search_regex(&dir, input, &idx);
				if (i >= 0) {
					cursor = idx;
					ui_status_info("");
				} else switch (-i) {
					case REGSEARCH_BAD_REGEX:
						ui_status_error("Bad RegEx");
						break;
					case REGSEARCH_NOT_FOUND:
						ui_status_info("No Matches Found");
						break;
					default:
						ui_status_error("Unhandled Case");
						break;
				}
				break;
			}
			case 'q':
				goto finished;
			case KEY_RESIZE:
				ui_resize();
				break;
			default: break;
		}
	}

finished:
	dir_free(&dir);
	ui_deinit();
}
