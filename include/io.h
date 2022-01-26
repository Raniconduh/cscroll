#ifndef _IO_H
#define _IO_H

#include <stdbool.h>
#include <stdio.h>

#include "dir.h"

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
	CTRL_B = 2,
	CTRL_C = 3,
	CTRL_F = 6,
	CTRL_N = 14,
	CTRL_P = 16,
	CTRL_Z = 26,
};


void curses_init(void);
void terminate_curses(void);
void curses_write_file(struct dir_entry_t *, bool);
char * prompt(char *, char **);
char * curses_getline(char *);
void unmark_all(void);
void mark_all(void);
void set_color(void);


extern bool print_path;
extern int stdout_back;
extern size_t n_marked_files;

#endif /* _IO_H */
