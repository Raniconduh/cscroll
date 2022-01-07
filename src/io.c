#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#if ICONS
#include "icons.h"
#include "type.h"
#endif
#include "opts.h"
#include "dir.h"
#include "io.h"


bool print_path = false;
int stdout_back = 0;
size_t n_marked_files = false;


void curses_init(void) {
	if (print_path) {
		stdout_back = dup(STDOUT_FILENO);
		dup2(open("/dev/tty", O_WRONLY), STDOUT_FILENO);
	}

	initscr();
	curs_set(0);
	noecho();
	raw();
	
	start_color();
	set_color();
}


void terminate_curses(void) {
	curs_set(1);
	echo();
	noraw();
	endwin();
	
	if (print_path) {
		dup2(stdout_back, STDOUT_FILENO);
	}
}


void set_color(void) {
	if (color) {
		init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
		init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
		init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
		init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
		init_pair(RED, COLOR_RED, COLOR_BLACK);
		init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
	} else {
		init_pair(BLUE, COLOR_WHITE, COLOR_BLACK);
		init_pair(CYAN, COLOR_WHITE, COLOR_BLACK);
		init_pair(GREEN, COLOR_WHITE, COLOR_BLACK);
		init_pair(MAGENTA, COLOR_WHITE, COLOR_BLACK);
		init_pair(YELLOW, COLOR_WHITE, COLOR_BLACK);
		init_pair(RED, COLOR_WHITE, COLOR_BLACK);
		init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
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
	char * owner = NULL;
	char * group = NULL;
	char * size  = NULL;
	char time[128];
	
	if (p_long) {
		smode = mode_to_s(dir_entry);
		owner = getpwuid(dir_entry->owner)->pw_name;
		group = getgrgid(dir_entry->group)->gr_name;
		switch (dir_entry->u_size) {
			case 0: size = "B"; break;
			case 1: size = "KB"; break;
			case 2: size = "MB"; break;
			case 3: size = "GB"; break;
			case 4: size = "TB"; break;
			case 5: size = "PB"; break;
		}
		strftime(time, sizeof(time), "%b %d %H:%M %Y",
				localtime(&dir_entry->mtime));
	}

	switch (dir_entry->file_type) {
		case FILE_DIR:
			cp = BLUE;
			f_ident = '/';
			break;
		case FILE_FIFO:
			cp = YELLOW;
			f_ident = '|';
			break;
		case FILE_BLK:
			cp = YELLOW;
			f_ident = '#';
			break;
		case FILE_LINK:
			if (dir_entry->under_link == FILE_DIR)
				u_text = "=> /";
			cp = CYAN;
			f_ident = '@';
			break;
		case FILE_SOCK:
			cp = MAGENTA;
			f_ident = '=';
			break;
		case FILE_UNKNOWN:
			cp = RED;
			f_ident = '?';
			break;
		default:
			f_ident = NO_IDENT;
			break;
	}

	switch (dir_entry->m_type) {
		case MIME_MEDIA:
			cp = MAGENTA;
#if ICONS
			icon = ICON_VIDEO;
#endif
			break;
		case MIME_ARCHIVE:
			cp = RED;
#if ICONS
			icon = ICON_ARCHIVE;
#endif
			break;
		case MIME_UNKNOWN:
		default:
			break;
	}

#if ICONS
	// find icon if it is not media or an archive
	if (!icon && show_icons) {
		char * t_ext = get_ext(dir_entry->name);
		if (t_ext && *t_ext) {
			char * ext = malloc(strlen(t_ext));
			strcpy(ext, t_ext);
			lowers(ext);
			struct icon_pair * t_icon =
				bsearch(&ext, icons, n_icons, sizeof(icons[0]), icmp);
			if (t_icon) icon = t_icon->icon;
			free(ext);
		}
	}

	if (!icon && dir_entry->file_type == FILE_DIR) icon = ICON_DIR;
#endif

	if ((dir_entry->mode & POWNER(M_EXEC)) &&
			dir_entry->file_type != FILE_LINK &&
			dir_entry->file_type != FILE_DIR) {
		cp = GREEN;
		if (f_ident == NO_IDENT) f_ident = '*';
#if ICONS
		if (!icon) icon = ICON_GEAR;
#endif
	} else if (cp == -1) {
		cp = WHITE;
		f_ident = ' ';
	}

#if ICONS
	if (!icon) icon = ICON_GENERIC;
#endif

	cp = COLOR_PAIR((unsigned)cp);
	if (highlight) cp |= A_REVERSE;

	if (dir_entry->marked) printw("%c ", '-');
	if (p_long) {
		printw("%s %s %s %-4d%2s %s ",
				smode, owner, group,
				dir_entry->size, size, time);
		free(smode);
	}
#if ICONS
	if (show_icons) printw("%s ", icon);
#endif
	attron(cp);
	printw("%s", dir_entry->name);
	attroff(cp);
	printw("%c %s\n", f_ident, u_text);
}


char curses_getch(void) {
	char c = getch();

	char seq[5] = {0};
	char * ptr = seq;

	if (c == 27) {
		char c = getch();
		if (c == '[') {
			*ptr++ = c;
		} else {
			ungetch(c);
			return seq[0];
		}
		*ptr++ = getch();
		*ptr++ = '\0';
	} else {
		switch (c) {
			case 2: return CTRL_B; break;
			case 6: return CTRL_F; break;
			case 14: return CTRL_N; break;
			case 16: return CTRL_P; break;
			default: break;
		}
	}

	if (seq[0] == '[')
		switch (seq[1]) {
			case 'A':
				return ARROW_UP;
			case 'B':
				return ARROW_DOWN;
			case 'D':
				return ARROW_LEFT;
			case 'C':
				return ARROW_RIGHT;
			default:
				break;
		}

	return c;
}


char * prompt(char * t, char ** args) {
	int sub_cols = 30;
	int sub_rows = sub_cols / 2;
	// newwin(rows, cols, y, x)
	WINDOW * w = newwin(sub_rows, sub_cols, LINES / 2 - sub_rows / 2, COLS / 2 - sub_cols / 2);
	werase(w);
	
	box(w, 0, 0);

	// print text strinf
	size_t tlen = strlen(t);
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

		char c = curses_getch();
		switch (c) {
			case ARROW_UP:
			case ARROW_LEFT:
			case CTRL_P:
			case CTRL_B:
			case 'h':
			case 'k':
				if (cursor > 1) cursor--;
				break;
			case ARROW_DOWN:
			case ARROW_RIGHT:
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
	echo();
	noraw();

	if (p)
		printw("%s", p);
	refresh();

	char * inp = malloc(128);
	size_t l = 0;
	char c;
	while ((c = getch()) != '\n') {
		if (c == 127) {
			if (l > 0) l--;
			addch(' ');
		}
		inp[l++] = c;
	}
	inp[l] = '\0';

	curs_set(0);
	noecho();
	raw();

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
