#include <ncurses.h>
#include <stdbool.h>
#include <string.h>

#include "dir.h"
#include "io.h"

void curses_init(void) {
	initscr();
	curs_set(0);
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
			default: break;
		}
		if (cp == -1) {
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
			default: break;
		}
		if (cp == -1) {
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

	if (!strcmp(seq, "[A"))
		return ARROW_UP;
	else if (!strcmp(seq, "[B"))
		return ARROW_DOWN;
	else
		return c;
}
