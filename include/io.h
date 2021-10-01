#ifndef IO_H
#define IO_H
#endif

#include <stdbool.h>

enum colors {
	BLUE,
	HBLUE,
	CYAN,
	HCYAN,
	GREEN,
	HGREEN,
	MAGENTA,
	HMAGENTA,
	YELLOW,
	HYELLOW,
	RED,
	HRED,
	WHITE,
	HWHITE,
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
