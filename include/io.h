#ifndef IO_H
#define IO_H
#endif

#include <stdbool.h>
#include <stdio.h>

#define NO_IDENT 0

enum colors {
	BLUE = 1,
	CYAN = 2,
	GREEN = 3,
	MAGENTA = 4,
	YELLOW = 5,
	RED = 6,
	WHITE = 7,
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
char * curses_getline(char *);
void unmark_all(void);
void mark_all(void);

extern bool print_path;
extern FILE * stdout_back;
extern size_t n_marked_files;
