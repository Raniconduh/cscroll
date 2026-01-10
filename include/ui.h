#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>

#include "dir.h"

#define CTRL(k) ((k) & 0x1F)

#define KEY_DEL 127
#define KEY_ESC 27

#define UP_KEYS \
	case KEY_UP:    \
	case CTRL('P'): \
	case 'k'

#define DOWN_KEYS \
	case KEY_DOWN:  \
	case CTRL('N'): \
	case 'j'

#define LEFT_KEYS \
	case KEY_LEFT:  \
	case CTRL('B'): \
	case 'h'

#define RIGHT_KEYS \
	case KEY_RIGHT: \
	case CTRL('F'): \
	case 'l'

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

// basically an array of strings (last entry is NULL)
typedef const char * const prompt_opts_t[];

void ui_init(void);
void ui_deinit(void);
void ui_reinit(void);
void ui_set_title(const char * title);
void ui_status_info(const char * status);
void ui_status_error(const char * status);
void ui_erase(void);
void ui_refresh(void);
void ui_print_dir(const dir_t * dir, size_t cursor);
void ui_print_cursor(size_t cursor, size_t total);
void ui_resize(void);
const char * ui_readline(const char * prompt);
const char * ui_prompt(const char * prompt, prompt_opts_t opts);
bool ui_prompt_deletion(const dirent_t * de);

#endif /* UI_H */
