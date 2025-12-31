#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>

#include "dir.h"

enum ui_color {
	COLOR_FILE = 1,
	COLOR_DIR,
	COLOR_FIFO,
	COLOR_LINK,
	COLOR_BLOCK,
	COLOR_CHAR,
	COLOR_SOCKET,
	COLOR_UNKNOWN,
	COLOR_EXEC,

	RED,
	YELLOW,
	MAGENTA,
	WHITE,

};

void ui_init(void);
void ui_deinit(void);
void ui_set_title(const char * title);
void ui_set_status(const char * status);
void ui_erase(void);
void ui_refresh(void);
void ui_print_dir(const dir_t * dir, size_t cursor, bool longmode);
void ui_print_cursor(size_t cursor, size_t total);

#endif /* UI_H */
