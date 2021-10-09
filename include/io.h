#ifndef IO_H
#define IO_H
#endif

#include <stdbool.h>

enum colors {
	BLUE = 1,
	HBLUE = 2,
	CYAN = 3,
	HCYAN = 4,
	GREEN = 5,
	HGREEN = 6,
	MAGENTA = 7,
	HMAGENTA = 8,
	YELLOW = 9,
	HYELLOW = 10,
	RED = 11,
	HRED = 12,
	WHITE = 13,
	HWHITE = 14,
};

enum keys {
	ARROW_UP,
	ARROW_DOWN,
	ARROW_LEFT,
	ARROW_RIGHT
};

void curses_init(void);
void terminate_curses(void);
void curses_write_file(struct dir_entry_t *, bool);
char curses_getch(void);
char * prompt(char *, char **);
