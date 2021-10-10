#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "dir.h"
#include "io.h"

bool print_path = false;
FILE * stdout_back = NULL;

void curses_init(void) {
	if (print_path) {
		stdout_back = stdout;
		stdout = fopen("/dev/tty", "w");
	}

	initscr();
	curs_set(0);
	noecho();
	start_color();
	
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(HBLUE, COLOR_BLACK, COLOR_BLUE);
    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(HCYAN, COLOR_BLACK, COLOR_CYAN);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(HGREEN, COLOR_BLACK, COLOR_GREEN);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(HMAGENTA, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(HYELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(HRED, COLOR_BLACK, COLOR_RED);
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(HWHITE, COLOR_BLACK, COLOR_WHITE); 
}


void terminate_curses(void) {
	curs_set(1);
	echo();
	endwin();
	
	if (print_path) {
		fclose(stdout);
		stdout = stdout_back;
	}
}


void curses_write_file(struct dir_entry_t * dir_entry, bool highlight) {
	int cp = -1;
	
	char f_ident;
	if (highlight) {
		switch (dir_entry->file_type) {
			case FILE_DIR:
				cp = HBLUE;
				f_ident = '/';
				break;
			case FILE_FIFO:
				cp = HYELLOW;
				f_ident = '|';
				break;
			case FILE_BLK:
				cp = HYELLOW;
				f_ident = '#';
				break;
			case FILE_LINK:
				cp = HCYAN;
				f_ident = '@';
				break;
			case FILE_SOCK:
				cp = HMAGENTA;
				f_ident = '=';
				break;
			case FILE_UNKNOWN:
				cp = HRED;
				f_ident = '?';
				break;
			default:
				f_ident = NO_IDENT;
				break;
		}
		if (dir_entry->exec) {
			cp = HGREEN;
			if (f_ident == NO_IDENT) f_ident = '*';
		} else if (cp == -1) {
			cp = HWHITE;
			f_ident = ' ';
		}
	} else {
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
		if (dir_entry->exec) {
			cp = GREEN;
			if (f_ident == NO_IDENT) f_ident = '*';
		} else if (cp == -1) {
			cp = WHITE;
			f_ident = ' ';
		}
	}

	attron(COLOR_PAIR((unsigned)cp));
	printw("%s", dir_entry->name);
	attroff(COLOR_PAIR((unsigned)cp));
	printw("%c\n", f_ident);
}


char curses_getch(void) {
	char c = getch();

	char seq[5];
	char * ptr = seq;

	if (c == 27) {
		*ptr++ = getch();
		*ptr++ = getch();
		*ptr++ = '\0';
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
				wattron(w, COLOR_PAIR(HWHITE));
			mvwprintw(w, sub_rows - 2, col + (sub_cols / 2 - argstrlen), "%s", args[i]);
			if ((unsigned)cursor - 1 == i)
				wattroff(w, COLOR_PAIR(HWHITE));
			col += strlen(args[i]) + 2;
		}

		wrefresh(w);

		char c = curses_getch();
		switch (c) {
			case ARROW_UP:
			case ARROW_LEFT:
			case 'h':
			case 'k':
				if (cursor > 1) cursor--;
				break;
			case ARROW_DOWN:
			case ARROW_RIGHT:
			case 'l':
			case 'j':
				if ((unsigned)cursor < argcount) cursor++;
				break;
			case '\n':
				delwin(w);
				return args[cursor - 1];
			case 'q':
			case 27:
				goto done;
		}
	}
done:;
	 delwin(w);
	 return NULL;
}
