#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <signal.h>

#include "commands.h"
#include "main.h"
#include "opts.h"
#include "dir.h"
#include "var.h"
#include "io.h"


size_t first_f = 0, last_f = 0, cursor = 0;


int main(int argc, char ** argv) {
	var_init();

	if (!check_config()) create_config();
	else read_config();
	terminate_opts();
	generate_colors();

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-p")) {
				print_path = true;
			} else if (!strcmp(argv[i], "-nc")) {
				color = false;
#if ICONS
			} else if (!strcmp(argv[i], "-ni")) {
				show_icons = false;
#endif
			} else if (!strcmp(argv[i], "-l")) {
				p_long = true;
			} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
				help();
			} else {
				cwd = realpath(argv[i], NULL);
				chdir(cwd);
			}
		}
	}

	if (!cwd) {
		char * p = getenv("PWD");
		cwd = malloc(strlen(p) + 2);
		strcpy(cwd, p);
	}

	curses_init();

	signal(SIGCONT, sig_handler);

	list_dir(cwd);

	cursor = 1;

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
		if (permission_denied) {
			attron(COLOR_PAIR(RED));
			printw("\tPermission Denied");
			attroff(COLOR_PAIR(RED));
		}
		if (cutting) printw("  cut");
		printw("\n\n");

		// print files
		for (size_t i = first_f; i < last_f; i++) {
			bool h = false;
			if (cursor - 1 == i)
				h = true;

			curses_write_file(dir_entries[i], h);
		}

		// print cursor / total entries
		if (n_dir_entries)
			printw("\n%lu/%lu\n", cursor, n_dir_entries);

		refresh();

		int c = getch();
		switch (c) {
			case KEY_UP:
			case CTRL_P:
			case 'k':
				if (cursor > 1) cursor--;
				break;
			case KEY_DOWN:
			case CTRL_N:
			case 'j':
				if (cursor < n_dir_entries) cursor++;
				break;
			case KEY_LEFT:
			case CTRL_B:
			case 'h':
				cd_back();
				free_dir_entries();
				list_dir(cwd);
				cursor = 1;
				first_f = 0;
				last_f = LAST_F;
				break;
			case KEY_RIGHT:
			case CTRL_F:
			case 'l':
			case '\n':
				if (!n_dir_entries) break;
				open_cur_file();
				break;
			case KEY_MOUSE:;
				MEVENT mouse_event;
				bool ok = false;
				unsigned norm_row = 0;  // normalized row
				if (getmouse(&mouse_event) == OK) {
					ok = true;
					int mrow = mouse_event.y; // row @ mouse click; start @ 1
					if (mrow < 3 || mrow > LINES - 4) break; // 3 pad top/bottom
					if ((unsigned)mrow - 3 >= n_dir_entries) break;

					norm_row = first_f + mrow - 2; // 3-pad, mrow>0
				}

				if (ok) {
					if (norm_row == cursor) {
						open_cur_file();
					} else {
						cursor = norm_row;
					}
				}

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
				char * args[] = {"No", "Yes", NULL};
				if (n_marked_files) {
					char * p = malloc(40);
					sprintf(p, "Remove all marked files? (%lu)", n_marked_files);
					char * resp = prompt(p, args);
					free(p);
					if (!resp || strcmp(resp, "Yes")) break;
					remove_marked();
					free_dir_entries();
					list_dir(cwd);
					cursor = 1;
					first_f = 0;
					last_f = LAST_F;
					break;
				}
				char * name = dir_entries[cursor - 1]->name;
				char * p = malloc(20 + strlen(name));
				sprintf(p, "Delete the file '%s'?", name);
				char * resp = prompt(p, args);
				free(p);
				if (!resp) break;
				if (!strcmp(resp, "Yes")) {
					p = malloc(strlen(cwd) + strlen(name) + 3);
					sprintf(p, "%s/%s", cwd, name);
					remove(p);
					free(p);
					free_dir_entries();
					list_dir(cwd);

					if (cursor > n_dir_entries) cursor--;
					last_f = LAST_F;
				}
				break;
			case 'm':
				// cannot mark files outside of start dir
				// while cutting
				if (cutting && strcmp(cwd, cut_start_dir)) break;

				if (!dir_entries[cursor - 1]->marked) {
					dir_entries[cursor - 1]->marked = true;
					n_marked_files++;
				} else {
					dir_entries[cursor - 1]->marked = false;
					if (n_marked_files) n_marked_files--;
				}
				break;
			case 'r':
				if (n_marked_files)
					break;
				char * nn = curses_getline(NULL); // new name
				if (strlen(nn) < 1) {
					free(nn);
					break;
				}
				// old path
				char * op = malloc(strlen(cwd) + strlen(dir_entries[cursor - 1]->name) + 2);
				// new path
				char * np = malloc(strlen(cwd) + strlen(nn) + 2);
				sprintf(op, "%s/%s", cwd, dir_entries[cursor - 1]->name);
				sprintf(np, "%s/%s", cwd, nn);
				rename(op, np);

				free(op);
				free(np);
				free(nn);

				free_dir_entries();
				list_dir(cwd);

				if (cursor > n_dir_entries) cursor--;
				else if (cursor < 1) cursor++;

				if (last_f > n_dir_entries) last_f--;

				break;
			case 'c':
				// stop cutting if pressed twice
				if (cutting) {
					free_cuts();
					break;
				}
				// copy cwd to start directory for cuts
				if (!n_marked_files) {
					char ** args = malloc(sizeof(char*) * 2);
					args[0] = malloc(strlen(dir_entries[cursor - 1]->name) + 1);
					strcpy(args[0], dir_entries[cursor - 1]->name);
					args[1] = NULL;
					create_cuts(cwd, args);
					free(args[0]);
					free(args);
				} else
					create_cuts(cwd, NULL);
				break;
			case 'p':
				if (!cutting) break;
				// only paste to different directories
				if (!strcmp(cwd, cut_start_dir)) {
					free_cuts();
					break;
				}

				if (n_marked_files) {
					char * args[] = {"No", "Yes", NULL};
					char * p = malloc(30);
					sprintf(p, "Paste all files (%lu)", n_marked_files);
					if (strcmp(prompt(p, args), "Yes")) {
						free(p);
						break;
					}
				}

				paste_cuts(cwd);
				free_cuts();

				free_dir_entries();
				list_dir(cwd);

				first_f = 0;
				last_f = LAST_F;
				cursor = 1;
				break;
			case ':':;
				char * inp = curses_getline(":");
				char * sp = strchr(inp, ' ');
				if (sp) {
					*sp = 0;
					char * cmd = inp;
					char * args = ++sp;
					if (!strcmp(cmd, "set")) set(args);
					else if (!strcmp(cmd, "var")) parse_var(args);
					else if (!strcmp(cmd, "unset")) unset(args);
				} else if (!strcmp(inp, "ma") && !cutting)
					mark_all();
				else if (!strcmp(inp, "mu")) {
					if (cutting) free_cuts();
					unmark_all();
				} else if (!strcmp(inp, "ca") && !cutting) {
					mark_all();
					create_cuts(cwd, NULL);
				}
				free(inp);
				break;
			case '/':;
				char * search_str = curses_getline("/");
				long c = search_file(cursor, search_str);
				free(search_str);

				if (c == -1) break;

				cursor = c + 1;

				// all files can fit on screen
				if (n_dir_entries <= (unsigned)LINES - 6) {
					first_f = 0;
					last_f = n_dir_entries;
				// somewhere in middle but no need to scroll
				} else if (cursor > first_f && cursor <= last_f) {
					; // no-op
				// somewhere in the middle
				} else if (cursor + LINES - 7 <= n_dir_entries) {
					first_f = c;
					last_f = first_f + LINES - 6;
				// at the end
				} else if (n_dir_entries > (unsigned)LINES - 6) {
					last_f = n_dir_entries;
					first_f = last_f - LINES + 6;
				}

				break;
			case '!':;
				char * cmd = curses_getline("!");
				if (!cmd[0]) break;
				// file name & length to sub
				char * f = dir_entries[cursor - 1]->name;
				size_t flen = strlen(f);

				char * cmdp = cmd; // tmp ptr to cmd
				char * f_sep;      // where %f is found
				size_t moves = 0;  // no. of times %f is found
				// replace '%f' with file name cursor is on
				while ((f_sep = strstr(cmdp, "%f"))) {
					// escape '%f' if '%%f' is found
					if (f_sep != cmd && f_sep[-1] == '%') {
						size_t seplen = strlen(f_sep);
						memmove(f_sep - 1, f_sep, seplen);
						f_sep[seplen - 1] = 0;
						cmdp = f_sep + 2;
						continue;
					}
					++moves;
					// store position of f_sep to be restored
					// after reallocating cmd
					ptrdiff_t t = f_sep - cmd;
					cmd = realloc(cmd, strlen(cmd) + flen * moves + 1);
					f_sep = cmd + t;
					cmdp = f_sep + 2;
					// make room for file name
					memmove(f_sep + flen, f_sep + 2, strlen(f_sep) - 1);
					// replace '%f' with file name
					for (size_t i = 0; i < flen; i++)
						*f_sep++ = f[i];
				}

				run_cmd(cmd);
				free(cmd);

				free_dir_entries();
				list_dir(cwd);

				resize_fbuf();

				break;
			case KEY_RESIZE:

				resize_fbuf();

				erase();
				refresh();

				break;
			case CTRL_Z:
				terminate_curses();
				// send self SIGSTOP -- restore shell feature
				kill(getpid(), SIGSTOP);
				break;
			case CTRL_C:
			case 'q':
				goto done;
			default:
				break;
		}
	}

done:
	free_dir_entries();
	free(dir_entries);
	terminate_var();
	terminate_curses();

	if (print_path) puts(cwd);

	return 0;
}


void help(void) {
	puts(
			"cscroll\n"
			"A small and efficient file manager\n"
			"\n"
			"Usage:\n"
			"  cscroll [OPTION]... [DIR]\n"
			"\n"
			"Options:\n"
			"  -h, --help          Show this screen and exit\n"
			"  -nc                 Turn off colors\n"
#if ICONS
			"  -ni                 Turn off icons\n"
#endif
			"  -l                  Print files in long mode\n"
			"  -p                  Print the path cscroll is in when it exits\n"
			"\n"
			"See https://github.com/Raniconduh/cscroll for documentation\n"
		);
	exit(0);
}


void sig_handler(int signo) {
	switch (signo) {
		case SIGCONT:
			curses_init();
			break;
	}
}
