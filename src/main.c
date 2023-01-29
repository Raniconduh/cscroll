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

# this is the main method
int main(int argc, char ** argv) {
	var_init();

	// config file needs home directory
	get_home();

	if (!check_config()) create_config();
	else read_config();
	terminate_opts();
	generate_colors();

	if (argc > 1) {
		bool parse_opts = true;
		bool negate_next = false;

		for (int i = 1; i < argc; i++) {
			if (parse_opts && argv[i][0] == '-') {
				if (argv[i][0] != '\0' && argv[i][1] == '-') {
					if (argv[i][2] == '\0') parse_opts = false;
					else if (!strcmp(argv[i] + 2, "help"))
						help();
					else if (!strcmp(argv[i] + 2, "oneshot"))
						oneshot = true;
					else {
						fprintf(stderr, "Unknown option: %s\n", argv[i]);
						exit(1);
					}
				} else {
					for (char * c = &argv[i][1]; *c; c++) {
						switch (*c) {
							case 'n':
								negate_next = true;
								break;
							case 'p':
								print_path = !negate_next;
								break;
							case 'c':
								color = !negate_next;
								break;
#if ICONS
							case 'i':
								show_icons = !negate_next;
								break;
#endif
							case 'l':
								p_long = !negate_next;
								break;
							case 'A':
								show_dot_files = !negate_next;
								break;
							case 'a':
								show_dot_files = !negate_next;
								show_dot_dirs = !negate_next;
								break;
							case 'h': break;
							default:
								fprintf(stderr, "Unknown option: -%c\n", *c);
								exit(1);
								break;
						}

						if (*c != 'n') negate_next = false;
					}
				}
			} else {
				cwd = realpath(argv[i], NULL);
				if (!cwd) cwd = strdup(argv[i]);
			}
		}
	}

	if (show_dot_dirs && !oneshot) {
		fputs("-a is only available in oneshot mode\n", stderr);
		exit(1);
	}

	if (!cwd) {
		char p[2048];
		getcwd(p, sizeof(p));
		cwd = malloc(strlen(p) + 2);
		strcpy(cwd, p);
	}

	bool cwd_is_dir = check_dpath(cwd);

	if (oneshot) {
		if (!cwd_is_dir) cwd_is_file = true;
		else list_dir(cwd);

		print_oneshot();

		free_dir_entries();
		free(dir_entries);
		terminate_var();
		free(cwd);
		exit(0);
	} else if (!cwd_is_dir) {
		fputs("Invalid directory specified\n", stderr);
		exit(1);
	} else {
		chdir(cwd);
	}

	if (!strncmp(cwd, homedir, homedir_len)) in_home_subdir = true;

	curses_init();
	info_init();

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

		// computer however many characters of cwd will be printed
		int cwd_len = 0;
		if (in_home_subdir) cwd_len = strlen(cwd) - homedir_len + 1;
		else cwd_len = strlen(cwd);

		// print cwd at top
		if (cwd_len >= COLS) {
			char * p = cwd + (cwd_len - COLS);
			if (in_home_subdir) {
				addch('~');
				p += homedir_len;
			}

			// there is not enough room for an elipsis,
			// just print whatever is left
			if (p + 5 > cwd + strlen(cwd)) {
				//addstr(p);
			} else {
				p += 5; // strlen("/...")
				addch('/');
				attron(A_DIM);
				addstr("...");
				attroff(A_DIM);
				addstr(p);
			}
		// write the full path regularly
		} else {
			char * p = cwd;
			if (in_home_subdir) {
				addch('~');
				p += homedir_len;
			}

			if (!*p) addch('/');
			else addstr(p);
		}

		addch('\n');
		bool printed_info = false;
		if (permission_denied) {
			printed_info = true;
			attron(COLOR_PAIR(RED));
			addstr("Permission Denied");
			attroff(COLOR_PAIR(RED));
		}
		if (cutting) {
			printed_info = true;
			if (permission_denied) padstr(2);
			addstr("Cut");
		}
		if (printed_info) addch('\n');
		addch('\n');

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
		refresh_info();

		int c = getch();
		switch (c) {
			case UP_KEYS:
				if (cursor > 1) cursor--;
				break;
			case DOWN_KEYS:
				if (cursor < n_dir_entries) cursor++;
				break;
			case LEFT_KEYS:
				// can't cd back when in /
				if (cwd[0] == '/' && cwd[1] == '\0') break;

				char * cur_dir = strdup(strrchr(cwd, '/') + 1);

				cd_back();
				free_dir_entries();
				list_dir(cwd);

				// set cursor to old dir
				long lc = search_file(0, cur_dir);
				if (lc >= 0) {
					cursor = lc + 1;
					resize_fbufcur(lc);
				} else {
					cursor = 1;
					first_f = 0;
					last_f = LAST_F;
				}

				free(cur_dir);
				break;
			case RIGHT_KEYS:
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
			case KEY_HOME:
			case 'g':
				cursor = 1;
				first_f = 0;
				last_f = LAST_F;
				break;
			case KEY_END:
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
				if (!nn) break;
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
				if (!inp) break;

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
				} else if (!strcmp(inp, "i")) {
					page_info();
				}
				free(inp);
				break;
			case '/':;
				char * search_str = curses_getline("/");
				if (!search_str) break;

				long c = search_file(cursor, search_str);
				free(search_str);

				if (c == -1) break;
				cursor = c + 1;

				resize_fbufcur(c);
				break;
			case '!':;
				char * cmd = curses_getline("!");
				if (!cmd) break;
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

	free(cwd);
	free(homedir);

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
			"  -A                  Show dotfiles except . and ..\n"
			"  -a                  Show all dotfiles, including . and ..\n"
			"                      Only available in oneshot mode\n"
			"  -c                  Use colors\n"
			"  -h                  No-op: for compatibility purposes\n"
#if ICONS
			"  -i                  Use icons\n"
#endif
			"  -l                  Print files in long mode\n"
			"  -n                  Negate the next flag\n"
			"  -p                  Print the path cscroll is in when it exits\n"
			"  --help              Show this screen and exit\n"
			"  --oneshot           Print and exit as if cscroll is ls\n"
			"  --                  Stop parsing '-' flags\n"
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
