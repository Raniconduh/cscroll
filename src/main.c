#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <ncurses.h>
#include <stdbool.h>

#include "ui.h"
#include "dir.h"
#include "config.h"
#include "cvector.h"

int main(int argc, char ** argv) {
	dir_init();
	const char * cwd = dir_get_cwd();
	// dir_get_home may return a static field so it must be copied here
	char * home = (char*)dir_get_home();
	if (home) home = strdup(home);

	dir_t dir;
	dir_list(cwd, &dir);

	ui_init();
	enum prog_mode mode = MODE_NORMAL;
	size_t cursor = 0;
	ui_status_info("");
	for (;;) {
		ui_erase();

		size_t dirlen = dir_len(&dir);

		ui_set_title(cwd, home);
		ui_print_dir(&dir, cursor);
		ui_print_cursor(cursor, dirlen, mode, dir_get_total_marked());

		ui_refresh();

		dirent_t * cur_de = dir_get_entry(&dir, cursor);

		int inputc = getch();
		if (inputc != KEY_RESIZE) ui_status_info("");
		switch (inputc) {
			UP_KEYS:
				if (cursor > 0) cursor--;
				break;
			DOWN_KEYS:
				if (cursor < dirlen - 1) cursor++;
				break;
			LEFT_KEYS: {
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
					} else if (basename) {
						size_t idx;
						int i = dir_search_name(&dir, basename, &idx);
						if (i >= 0) cursor = idx;
					}
				} else {
					ui_status_error(strerror(-ret));
				}
				free(basename);
				break;
			}
			RIGHT_KEYS: {
				if (!cur_de) break;
				if (cur_de->type == DE_DIR || (cur_de->type == DE_LINK
				  && cur_de->linktype == DE_DIR)) {
					int ret = dir_cd(cwd, cur_de->name);
					if (ret >= 0) {
						cursor = 0;
						cwd = dir_get_cwd();
						dir_free(&dir);
						dir_list(cwd, &dir);
						if (dir_len(&dir) == 0) ui_status_info("Empty Directory");
					} else {
						ui_status_error(strerror(-ret));
					}
				} else {
					ui_deinit();
					int ret = dirent_open(cur_de);
					if (ret < 0) ui_status_error(strerror(-ret));
					ui_reinit();
				}
				break;
			}
			case KEY_HOME:
			case 'g':
				cursor = 0;
				break;
			case KEY_END:
			case 'G':
				if (dirlen == 0) cursor = 0;
				else cursor = dirlen - 1;
				break;
			case '.': {
				config.dots = !config.dots;
				if (!cur_de) cursor = 0;
				else {
					size_t idx;
					int ret = dir_search_name(&dir, cur_de->name, &idx);
					if (ret < 0) cursor = 0;
					else cursor = idx;
				}
				break;
			}
			case 'o':
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
			case 'd': {
				if (mode == MODE_CUT) {
					ui_status_info("Cannot Delete While Cutting");
					break;
				}
				ui_refresh();
				bool del = ui_prompt_deletion(cur_de);
				if (!del) {
					ui_status_info("Not Deleting");
					ui_refresh();
					break;
				}

				ui_status_info("Deleting");
				ui_refresh();

				int ret = dirent_delete(cur_de);
				if (ret < 0) {
					ui_status_error("Deletion Failed");
				} else {
					ui_status_info("Deleted");
				}

				dir_free(&dir);
				dir_list(cwd, &dir);
				dirlen = dir_len(&dir);
				if (dirlen == 0) cursor = 0;
				else if (cursor > dirlen - 1) cursor = dirlen - 1;
				break;
			}
			case 'm': {
				if (!cur_de) break;
				if (mode == MODE_CUT) {
					ui_status_info("Cannot Mark While Cutting");
					break;
				}
				int ret = dirent_togglemark(cur_de);
				if (ret < 0) switch (-ret) {
					case TOGGLEMARK_PARENT_MARKED:
						ui_status_info("Ancestor Already Marked");
						break;
					case TOGGLEMARK_DIRENT_UNKNOWN:
						ui_status_error("Cannot Mark Unknown Dirent");
						break;
					default:
						ui_status_error("Mark Failed");
						break;
				}
				break;
			}
			case 'c':
				if (mode == MODE_NORMAL && dir_get_total_marked() > 0) mode = MODE_CUT;
				else if (mode == MODE_CUT) mode = MODE_NORMAL;
				break;
			case 'p': {
				if (mode != MODE_CUT) {
					ui_status_info("Cannot Paste if Not Cutting");
					break;
				}
				bool paste = ui_prompt_paste(dir_get_total_marked());
				if (!paste) {
					ui_status_info("Not Pasting");
					break;
				}
				mode = MODE_NORMAL;
				ui_status_info("Pasting");
				ui_refresh();

				size_t total_marks, total_pastes;
				total_marks = dir_get_total_marked();
				int ret = dir_paste_marks(cwd, &total_pastes);
				if (ret < 0) ui_status_error(strerror(-ret));
				else if (total_pastes == 0)
					ui_status_error("Nothing Pasted");
				else if (total_marks != total_pastes)
					ui_status_error("Not All Marks Pasted");
				else ui_status_info("Pasted");
				dir_free(&dir);
				dir_list(cwd, &dir);
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
	free(home);
	dir_free(&dir);
	ui_deinit();
	dir_deinit();
}
