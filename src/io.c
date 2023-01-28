#include <sys/ioctl.h>
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#if ICONS
#include "type.h"
#endif
#include "main.h"
#include "opts.h"
#include "dir.h"
#include "io.h"


bool print_path = false;
int stdout_back = 0;
size_t n_marked_files = false;

static int default_colors[] = {
	[COLOR_DIR]     = COLOR_BLUE,
	[COLOR_LINK]    = COLOR_CYAN,
	[COLOR_EXEC]    = COLOR_GREEN,
	[COLOR_SOCK]    = COLOR_MAGENTA,
	[COLOR_FIFO]    = COLOR_YELLOW,
	[COLOR_UNKNOWN] = COLOR_RED,
	[COLOR_FILE]    = COLOR_WHITE,
	[COLOR_BLOCK]   = COLOR_YELLOW,
	[COLOR_MEDIA]   = COLOR_MAGENTA,
	[COLOR_ARCHIVE] = COLOR_RED,
};


static char * ansi_colors[] = {
	[RED]           = ANSI_RED,
	[YELLOW]        = ANSI_YELLOW,
	[MAGENTA]       = ANSI_MAGENTA,
	[WHITE]         = ANSI_WHITE,

	[COLOR_DIR]     = ANSI_BLUE,
	[COLOR_LINK]    = ANSI_CYAN,
	[COLOR_EXEC]    = ANSI_GREEN,
	[COLOR_SOCK]    = ANSI_YELLOW,
	[COLOR_FIFO]    = ANSI_YELLOW,
	[COLOR_UNKNOWN] = ANSI_RED,
	[COLOR_FILE]    = ANSI_WHITE,
	[COLOR_BLOCK]   = ANSI_YELLOW,
	[COLOR_MEDIA]   = ANSI_MAGENTA,
	[COLOR_ARCHIVE] = ANSI_RED,
};


static char * size_strings[] = {
	"B", "KB", "MB", "GB", "TB", "PB"
};


void curses_init(void) {
	if (print_path) {
		stdout_back = dup(STDOUT_FILENO);
		dup2(open("/dev/tty", O_WRONLY), STDOUT_FILENO);
	}

	setlocale(LC_CTYPE, "");

	initscr();
	keypad(stdscr, true);
	mouseinterval(0);
	curs_set(0);
	noecho();
	raw();

	mousemask(BUTTON1_PRESSED, NULL);
	
	start_color();
	set_color();
}


void terminate_curses(void) {
	keypad(stdscr, false);
	curs_set(1);
	echo();
	noraw();
	endwin();
	
	if (print_path) {
		dup2(stdout_back, STDOUT_FILENO);
	}
}


void set_color(void) {
	generate_colors();
	bool cc = can_change_color();

	if (color) {
		for (int i = CUSTOM_DIR; i <= CUSTOM_ARCHIVE; i++) {
			int def = i - CUSTOM_DIR + 1; // default color / index
			if (custom_colors[def] == COLOR_DEFAULT || !cc) {
				init_pair(def, default_colors[def], COLOR_BLACK);
			} else {
				init_color(i, GET_RGB(custom_colors[def]));
				init_pair(def, i, COLOR_BLACK);
			}
		}

		init_pair(RED, COLOR_RED, COLOR_BLACK);
		init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
		init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
		init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	} else {
		// COLOR_ARCHIVE is highest enum value
		for (int i = 1; i <= COLOR_ARCHIVE; i++) {
			init_pair(i, COLOR_WHITE, COLOR_BLACK);
		}

		init_pair(RED, COLOR_WHITE, COLOR_BLACK);
		init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
		init_pair(YELLOW, COLOR_WHITE, COLOR_BLACK);
		init_pair(MAGENTA, COLOR_WHITE, COLOR_BLACK);
	}
}


void curses_write_file(struct dir_entry_t * dir_entry, bool highlight) {
	int cp = -1;
	char f_ident;
	char * u_text = "";
#if ICONS
	char * icon = NULL;
#endif
	char * smode = NULL;
	char * owner = get_oname(dir_entry->owner);
	char * group = get_gname(dir_entry->group);
	char * size  = NULL;
	char time[128];
	
	if (p_long) {
		smode = mode_to_s(dir_entry);
		size = size_strings[dir_entry->u_size];
		strftime(time, sizeof(time), "%b %d %H:%M %Y",
				localtime(&dir_entry->mtime));
	}

	cp = get_file_color(dir_entry);
	f_ident = get_file_ident(dir_entry);
	u_text = dir_entry->under_link == FILE_DIR ? "=> /" : "";

#if ICONS
	// find icon if it is not a dir
	if (show_icons) {
		icon = get_icon(dir_entry);
	}
#endif

	cp = COLOR_PAIR((unsigned)cp);
	if (highlight) cp |= A_REVERSE;

	if (dir_entry->marked) printw("%c ", '-');
	if (p_long) {
		print_mode(dir_entry);
		// print owner
		addch(' ');
		addstr(owner);
		size_t n = strlen(owner);
		if (n < dir_longest_owner) padstr(dir_longest_owner - n);

		// print group
		addch(' ');
		addstr(group);
		n = strlen(group);
		if (n < dir_longest_group) padstr(dir_longest_group - n);

		printw(" %4lu %-2s %s ",
				dir_entry->size, size, time);
		free(smode);
	}
#if ICONS
	if (show_icons) printw("%s ", icon);
#endif

	size_t name_len = strlen(dir_entry->name);
	size_t ext_len = 0;
	if (f_ident != NO_IDENT) ext_len++;
	if (*u_text) ext_len += strlen(u_text);

	int y, x;
	getyx(stdscr, y, x);
	(void)y;

	attron(cp);

	if (x + name_len + ext_len >= (unsigned)COLS) {
		// allow space for ident and u_text
		int padding = 4; // 3 for ..., 1 for space before NL
		padding += ext_len ? ext_len + 1 : 0;
		if (padding > COLS - x) padding = 4;

		addnstr(dir_entry->name, COLS - x - padding);
		attroff(cp);
		addstr("...");
	} else {
		printw("%s", dir_entry->name);
		attroff(cp);
	}

	printw("%c %s", f_ident, u_text);
	addch('\n');

	free(owner);
	free(group);
}


void print_mode(struct dir_entry_t * f) {
	enum colors m_colors[127] = {
		['s'] = YELLOW, ['d'] = COLOR_DIR, ['.'] = WHITE,
		['r'] = RED,    ['w'] = MAGENTA,   ['x'] = COLOR_EXEC,
		['S'] = YELLOW, ['t'] = RED,       ['T'] = RED,
		['-'] = WHITE,
	};

	char * mode = mode_to_s(f);
	for (char * c = mode; *c; c++) {
		if (oneshot) {
			if (color) {
				char * color = ansi_colors[m_colors[(int)*c]];
				printf("%s%c" ANSI_RESET, color, *c);
			} else {
				fputc(*c, stdout);
			}
		} else {
			bool dim = false;

			int cp = m_colors[(int)*c];
			if (!cp) cp = WHITE;
			if (cp == WHITE) dim = true;
			cp = COLOR_PAIR(cp);
			if (dim) cp |= A_DIM;
			attron(cp);
			addch(*c);
			attroff(cp);
		}
	}
	free(mode);
}


void padstr(size_t n) {
	if (oneshot) {
		for (size_t i = 0; i < n; i++) fputc(' ', stdout);
	} else {
		for (size_t i = 0; i < n; i++) addch(' ');
	}
}


char * prompt(char * t, char ** args) {
	size_t tlen = strlen(t);

	int sub_cols;
	// wider than 75% of the screen; need to break it down
	if (tlen > (unsigned)(COLS * 3) / 4) sub_cols = COLS * 3 / 4 - 1;
	// default is 4 wider than printed text (padding)
	else  sub_cols = tlen + 4;

	/**********
	 * calculate the number of lines the text will need
	 * 2 lines padding top/bottom
	 * lines_of_text + 1 for prompt
	 * extra line between text and options
	 * 2 lines for options
	 * 6 total
	 **********/
	size_t n_text_lines = tlen / sub_cols + 1; // +1 for int division
	int sub_rows = n_text_lines + 6;

	// newwin(rows, cols, y, x)
	WINDOW * w = newwin(sub_rows, sub_cols, LINES / 2 - sub_rows / 2, COLS / 2 - sub_cols / 2);
	werase(w);
	
	box(w, 0, 0);

	// print text string
	if (tlen < (unsigned)sub_cols - 2)
		mvwprintw(w, 2, sub_cols / 2 - tlen / 2, "%s", t);
	else {
		int row = 1;
		int col = 1;
		for (size_t i = 0; i < tlen; i++) {
			col++;
			if (i % (sub_cols - 4) == 0) {
				row++;
				col = 2;
			}
			mvwaddch(w, row, col, t[i]);
		}
	}

	if (!args) {
		wrefresh(w);
		napms(3500);
		delwin(w);
		return NULL;
	}

	size_t argcount = 0;
	size_t argstrlen = 0;
	for (char ** p = args; *p; p++) {
		argcount++;
		argstrlen += strlen(*p);
	}

	int cursor = 1;

	// pick option
	while (true) {
		int col = 2;
		for (size_t i = 0; i < argcount; i++) {
			if ((unsigned)cursor - 1 == i)
				wattron(w, COLOR_PAIR(WHITE) | A_REVERSE);
			mvwprintw(w, sub_rows - 2, col + (sub_cols / 2 - argstrlen), "%s", args[i]);
			if ((unsigned)cursor - 1 == i)
				wattroff(w, COLOR_PAIR(WHITE) | A_REVERSE);
			col += strlen(args[i]) + 2;
		}

		wrefresh(w);

		int c = getch();
		switch (c) {
			case KEY_UP:
			case KEY_LEFT:
			case CTRL_P:
			case CTRL_B:
			case 'h':
			case 'k':
				if (cursor > 1) cursor--;
				break;
			case KEY_DOWN:
			case KEY_RIGHT:
			case CTRL_N:
			case CTRL_F:
			case 'l':
			case 'j':
				if ((unsigned)cursor < argcount) cursor++;
				break;
			case '\n':
			case ' ':
				delwin(w);
				return args[cursor - 1];
			case 'q':
			case 27:
				goto done;
			default:
				break;
		}
	}
done:;
	 delwin(w);
	 return NULL;
}


char * curses_getline(char * p) {
	curs_set(1);

	size_t plen = 0;
	if (p) {
		addstr(p);
		plen = strlen(p);
		refresh();
	}

	char * inp = malloc(128);
	size_t l = 0;
	char c;
	while ((c = getch()) != '\n') {
		if ((l + 1) % 127 == 0) {
			// add 1 byte to prevent buffer overrun on deletion
			inp = realloc(inp, l + 129);
		}

		if (c == 127) {
			if (l > 0) l--;
		} else if (isprint(c)) {
			// do not bother with non-printable characters
			// line editing may be added later
			inp[l++] = c;
		} else continue;

		int y, x;
		getyx(stdscr, y, x);

		// it is time to scroll
		if (l >= COLS - plen - 1) {
			move(y, plen);
			for (size_t i = l - COLS + plen + 1; i < l; i++) {
				addch(inp[i]);
			}
		} else if ((unsigned)x != plen && c == 127) {
			mvaddch(y, x - 1, ' ');
			move(y, x - 1);
		} else if (c != 127) {
			addch(c);
		}

		refresh();
	}

	if (l == 0) {
		free(inp);
		inp = NULL;
	} else inp[l] = '\0';
	curs_set(0);

	return inp;
}


void mark_all(void) {
	for (size_t i = 0; i < n_dir_entries; i++)
		dir_entries[i]->marked = true;
	n_marked_files = n_dir_entries;
}


void unmark_all(void) {
	for (size_t i = 0; i < n_dir_entries; i++)
		dir_entries[i]->marked = false;
	n_marked_files = 0;
}


void resize_fbuf(void) {
	if (n_dir_entries <= (unsigned)LINES - 6) {
		first_f = 0;
		last_f = n_dir_entries;
	} else if (LINES <= 6) {
		if (first_f + 1 < n_dir_entries) last_f = first_f + 1;
		else last_f = first_f;
	} else if ((unsigned)LINES - 6 > n_dir_entries) {
		last_f = n_dir_entries;
	} else {
		last_f = first_f + LINES - 6;
	}

	if (cursor > last_f + 1) cursor = last_f + 1;

}


void resize_fbufcur(long c) {
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
}


int get_fwidth(struct dir_entry_t * de) {
	int w = 0;
	w += strlen(de->name);
#if ICONS
	if (show_icons) w += 2;
#endif

	// file ident
	w += 1;

	return w;
}


void print_oneshot(void) {
	if (cwd_is_file) {
		char * end_sep = strrchr(cwd, '/');
		char * f_name;
		char * wd;
		bool malloc_wd = false;

		if (end_sep) {
			*end_sep = '\0';
			f_name = end_sep + 1;
			wd = cwd;
		} else {
			f_name = cwd;
			malloc_wd = true;
			wd = malloc(128);
			getcwd(wd, 128);
		}

		struct dir_entry_t * de = gen_dir_entry(wd, f_name);
		if (malloc_wd) free(wd);
		if (!de) {
			fputs("No such file or directory\n", stderr);
			free(cwd);
			exit(1);
		}

		n_dir_entries = 1;
		dir_entries = malloc(sizeof(struct dir_entry_t));
		dir_entries[0] = de;
	}

	if (p_long) {
		char * icon = "";
		char *owner, *group, *size;
		char time[128];
		for (size_t i = 0; i < n_dir_entries; i++) {
			struct dir_entry_t * de = dir_entries[i];

			char * fcolor;
			if (color) {
				enum colors cp = get_file_color(de);
				if (cp == COLOR_WHITE) fcolor = "";
				else fcolor = ansi_colors[cp];
			} else {
				fcolor = "";
			}
			char f_ident = get_file_ident(de);
			char * u_text = de->under_link == FILE_DIR ? "=> /" : "";

#if ICONS
			if (show_icons) icon = get_icon(de);
#endif
			size = size_strings[de->u_size];
			strftime(time, sizeof(time), "%b %d %H:%M %Y",
					localtime(&de->mtime));
			owner = getpwuid(de->owner)->pw_name;
			group = getgrgid(de->group)->gr_name;

			// print mode
			print_mode(de);

			// print owner
			fputc(' ', stdout);
			fputs(owner, stdout);
			size_t n = strlen(owner);
			if (n < dir_longest_owner) padstr(dir_longest_owner - n);

			// print group
			fputc(' ', stdout);
			fputs(group, stdout);
			n = strlen(group);
			if (n < dir_longest_owner) padstr(dir_longest_owner - n);

			// print rest of the long mode info
			printf(" %4lu %-2s %s %s%s %s%s%c %s\n",
					de->size, size, time, fcolor, icon, de->name,
					ANSI_RESET, f_ident, u_text);
		}
	} else { // regular printing mode
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		int t_width = w.ws_col;

		int col_widths[127] = {0};
		int max_cols = sizeof(col_widths) + 1;
		int cur_col = 0;
		int line_width = 0;

		struct dir_entry_t * prev = NULL;

		// print until screen row is file
		// or until 128 colums of files have been used
		// first calculate padding
		for (size_t i = 0; i < n_dir_entries; i++) {
			struct dir_entry_t * de = dir_entries[i];

			int fwidth = get_fwidth(de);

			// reached max columns
			// or file with + padding > terminal width
			if (cur_col == max_cols || (line_width && line_width + fwidth + 2 >= t_width)) {
				cur_col = 0;
				line_width = 0;

				prev->last_in_col = true;
			}

			if (fwidth > col_widths[cur_col]) {
				col_widths[cur_col] = fwidth;
			}

			line_width += col_widths[cur_col] + 2;
			cur_col++;

			prev = de;
		}

		cur_col = 0;

		bool neednl = false;

		// print files
		for (size_t i = 0; i < n_dir_entries; i++) {
			struct dir_entry_t * de = dir_entries[i];

			char * fcolor = "";
			char * creset = "";

			if (color) {
				fcolor = ansi_colors[get_file_color(de)];
				creset = ANSI_RESET;
			}

			char * icon = NULL;
			char ident = get_file_ident(de);

			int fwidth = get_fwidth(de);


#if ICONS
			if (show_icons) icon = get_icon(de);
#endif

			if (icon) {
				fputs(icon, stdout);
				putchar(' ');
			}

			printf("%s%s%s", fcolor, de->name, creset);
			if (ident == NO_IDENT) ident = ' ';
			putchar(ident);

			if (!de->last_in_col) {
				fputs("  ", stdout);
				// string padding
				for (int i = 0; i < col_widths[cur_col] - fwidth; i++)
					putchar(' ');
			}

			cur_col++;

			if (de->last_in_col) {
				cur_col = 0;
				putchar('\n');
				neednl = false;
			} else neednl = true;

		}

		if (neednl) putchar('\n');
	}
}


enum colors get_file_color(struct dir_entry_t * de) {
	int cp = -1;

	switch (de->file_type) {
		case FILE_DIR:
			cp = COLOR_DIR;     break;
		case FILE_FIFO:
			cp = COLOR_FIFO;    break;
		case FILE_BLK:
			cp = COLOR_BLOCK;   break;
		case FILE_LINK:
			cp = COLOR_LINK;    break;
		case FILE_SOCK:
			cp = COLOR_SOCK;    break;
		case FILE_UNKNOWN:
			cp = COLOR_UNKNOWN; break;
		case FILE_REG:
			cp = COLOR_FILE;    break;
	}

	switch (de->m_type) {
		case MIME_MEDIA:
			cp = COLOR_MEDIA;    break;
		case MIME_ARCHIVE:
			cp = COLOR_ARCHIVE;  break;
		case MIME_UNKNOWN:
		default: break;
	}

	if ((de->mode & POWNER(M_EXEC)) &&
			de->file_type != FILE_LINK &&
			de->file_type != FILE_DIR) {
		cp = COLOR_EXEC;
	} else if (cp == -1) {
		cp = COLOR_FILE;
	}

	return (enum colors)cp;
}


char get_file_ident(struct dir_entry_t * de) {
	char f_ident;

	switch (de->file_type) {
		case FILE_DIR:
			f_ident = '/';
			break;
		case FILE_FIFO:
			f_ident = '|';
			break;
		case FILE_BLK:
			f_ident = '#';
			break;
		case FILE_LINK:
			f_ident = '@';
			break;
		case FILE_SOCK:
			f_ident = '=';
			break;
		case FILE_UNKNOWN:
			f_ident = '?';
			break;
		default:
			f_ident = NO_IDENT;
			break;
	}

	if ((de->mode & POWNER(M_EXEC)) &&
			de->file_type != FILE_LINK &&
			de->file_type != FILE_DIR &&
			f_ident == NO_IDENT) {
		f_ident = '*';
	}

	return f_ident;
}


size_t get_ilen(long i, int base) {
	size_t l = 0;

	while (i > 0) {
		i /= base;
		l++;
	}

	return l;
}


char * get_oname(uid_t uid) {
	char * buf = NULL;
	struct passwd * pw = getpwuid(uid);
	if (!pw) {
		buf = malloc(get_ilen(uid, 10) + 1);
		sprintf(buf, "%d", uid);
	} else {
		buf = malloc(strlen(pw->pw_name) + 1);
		strcpy(buf, pw->pw_name);
	}
	return buf;
}


char * get_gname(gid_t gid) {
	char * buf = NULL;
	struct group * gr = getgrgid(gid);
	if (!gr) {
		buf = malloc(get_ilen(gid, 10) + 1);
		sprintf(buf, "%d", gid);
	} else {
		buf = malloc(strlen(gr->gr_name) + 1);
		strcpy(buf, gr->gr_name);
	}
	return buf;
}
