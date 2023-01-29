#ifndef _IO_H
#define _IO_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "dir.h"

#define NO_IDENT 0


#define ESC "\033"
#define ANSI_RED      ESC "[31m"
#define ANSI_GREEN    ESC "[32m"
#define ANSI_YELLOW   ESC "[33m"
#define ANSI_BLUE     ESC "[34m"
#define ANSI_MAGENTA  ESC "[35m"
#define ANSI_CYAN     ESC "[36m"
#define ANSI_WHITE    ESC "[37m"
#define ANSI_RESET    ESC "[0m"


enum colors {
	COLOR_DIR     = 1,
	COLOR_LINK    = 2,
	COLOR_EXEC    = 3,
	COLOR_SOCK    = 4,
	COLOR_FIFO    = 5,
	COLOR_UNKNOWN = 6,
	COLOR_FILE    = 7,
	COLOR_BLOCK   = 8,
	COLOR_MEDIA   = 9,
	COLOR_ARCHIVE = 10,

	CUSTOM_DIR    = 11,
	CUSTOM_LINK   = 12,
	CUSTOM_EXEC   = 13,
	CUSTOM_SOCK   = 14,
	CUSTOM_FIFO   = 15,
	CUSTOM_UNKNOWN = 16,
	CUSTOM_FILE   = 17,
	CUSTOM_BLOCK  = 18,
	CUSTOM_MEDIA  = 19,
	CUSTOM_ARCHIVE = 20,

	RED     = 21,
	WHITE   = 22,
	YELLOW  = 23,
	MAGENTA = 24,
};

enum keys {
	CTRL_B = 2,
	CTRL_C = 3,
	CTRL_F = 6,
	CTRL_N = 14,
	CTRL_P = 16,
	CTRL_Z = 26,
};

enum info_t {
	INFO_INFO,
	INFO_WARN,
	INFO_ERR,
};


struct info_node {
	enum info_t type;
	char * msg;
};


void curses_init(void);
void terminate_curses(void);
void curses_write_file(struct dir_entry_t *, bool);
char * prompt(char *, char **);
char * curses_getline(char *);
void unmark_all(void);
void mark_all(void);
void set_color(void);
void print_mode(struct dir_entry_t *);
void padstr(size_t);
void resize_fbuf(void);
void resize_fbufcur(long);
void print_oneshot(void);
enum colors get_file_color(struct dir_entry_t *);
char get_file_ident(struct dir_entry_t *);
size_t get_ilen(long, int);
char * get_oname(uid_t);
char * get_gname(gid_t);
void info_init(void);
void display_info(enum info_t, char *);
void refresh_info(void);


extern bool print_path;
extern int stdout_back;
extern size_t n_marked_files;

#endif /* _IO_H */
