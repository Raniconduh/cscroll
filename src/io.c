#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dir.h"
#include "io.h"

bool print_path = false;
FILE * stdout_back = NULL;
size_t n_marked_files = false;

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
	init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(RED, COLOR_RED, COLOR_BLACK);
	init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
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
	char * u_text = "";

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

	if (dir_entry->exec && dir_entry->file_type != FILE_LINK) {
		cp = GREEN;
		if (f_ident == NO_IDENT) f_ident = '*';
	} else if (cp == -1) {
		cp = WHITE;
		f_ident = ' ';
	}
	
	cp = COLOR_PAIR((unsigned)cp);
	if (highlight) cp |= A_REVERSE;

	if (dir_entry->marked) printw("%c ", '-');
	attron(cp);
	printw("%s", dir_entry->name);
	attroff(cp);
	printw("%c %s\n", f_ident, u_text);
}


char curses_getch(void) {
	char c = getch();

	char seq[5];
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

	printw("%s", p);
	refresh();

	char * inp = malloc(128);
	size_t l = 0;
	char c;
	while ((c = getch()) != '\n')
		inp[l++] = c;

	curs_set(0);
	noecho();

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
