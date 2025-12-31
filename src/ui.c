#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <locale.h>
#include <time.h>

#include "ui.h"
#include "dir.h"

static WINDOW * titlewin = NULL;
static WINDOW * statuswin = NULL;
static WINDOW * filewin = NULL;

static void win_set(WINDOW * win, const char * str, int attrs);
static int ui_dirent_color(const dirent_t * de);
static void ui_print_dirent(const dirent_t * de, size_t pos, bool selected, bool longmode, const dir_t * dir);
static void ui_wlpadstr(WINDOW * win, const char * s, size_t len);
static void ui_get_first_last(size_t n_dirents, size_t cursor, size_t * first, size_t * last);

void ui_init(void) {
	setlocale(LC_CTYPE, "");

	initscr();
	keypad(stdscr, TRUE);
	curs_set(0);
	noecho();

	titlewin  = newwin(1, COLS, 0, 0);
	statuswin = newwin(1, COLS, 1, 0);
	filewin   = newwin(LINES - 2, COLS, 2, 0);

	clear();
	refresh();

	use_default_colors();
	start_color();

	init_pair(COLOR_FILE,    -1,            -1);
	init_pair(COLOR_DIR,     COLOR_BLUE,    -1);
	init_pair(COLOR_FIFO,    COLOR_YELLOW,  -1);
	init_pair(COLOR_LINK,    COLOR_CYAN,    -1);
	init_pair(COLOR_BLOCK,   COLOR_YELLOW,  -1);
	init_pair(COLOR_CHAR,    COLOR_YELLOW,  -1);
	init_pair(COLOR_SOCKET,  COLOR_MAGENTA, -1);
	init_pair(COLOR_UNKNOWN, COLOR_RED,     -1);
	init_pair(COLOR_EXEC,    COLOR_GREEN,   -1);

	init_pair(RED,     COLOR_RED,     -1);
	init_pair(YELLOW,  COLOR_YELLOW,  -1);
	init_pair(MAGENTA, COLOR_MAGENTA, -1);
	init_pair(WHITE,   -1,            -1);
}

void ui_deinit(void) {
	endwin();
}

void win_set(WINDOW * win, const char * str, int attrs) {
	werase(win);
	wattron(win, attrs);
	waddstr(win, str);
	wattroff(win, attrs);
}

void ui_set_title(const char * title) {
	win_set(titlewin, title, 0);
}

void ui_set_status(const char * status) {
	win_set(statuswin, status, 0);
}

void ui_erase(void) {
	werase(filewin);
}

void ui_refresh(void) {
	wnoutrefresh(titlewin);
	wnoutrefresh(statuswin);
	wnoutrefresh(filewin);
	doupdate();
}

static void ui_wlpadstr(WINDOW * win, const char * s, size_t len) {
	size_t slen = strlen(s);

	for (size_t i = slen; i < len; i++) {
		waddch(win, ' ');
	}

	waddstr(win, s);
}

void ui_print_dirent(const dirent_t * de, size_t pos, bool selected, bool longmode, const dir_t * dir) {
	static const enum ui_color mode_colors[] = {
		['r'] = RED,    ['w'] = MAGENTA, ['x'] = COLOR_EXEC,
		['s'] = YELLOW, ['S'] = YELLOW,  ['t'] = RED,
		['T'] = RED,    ['-'] = WHITE,   ['?'] = RED,

		['.'] = WHITE,
		['d'] = COLOR_DIR,  ['b'] = COLOR_BLOCK, ['c'] = COLOR_CHAR,
		['l'] = COLOR_LINK, ['|'] = COLOR_FIFO,  ['='] = COLOR_SOCKET,
	};

	static char stime[128];
	static char isbuf[40]; // should be big enough to hold any integer

	wmove(filewin, pos, 0);

	if (longmode) {
		const char * pmode = dirent_prettymode(de);
		for (const char * p = pmode; *p; p++) {
			int color = COLOR_PAIR(mode_colors[(unsigned)*p]);
			if (*p == '-' || *p == '.') color |= A_DIM;
			wattron(filewin, color);
			waddch(filewin, *p);
			wattroff(filewin, color);
		}

		if (de->uname) ui_wlpadstr(filewin, de->uname, dir->longest_uname + 1);
		else waddstr(filewin, " ?");

		if (de->gname) ui_wlpadstr(filewin, de->gname, dir->longest_gname + 1);
		else waddstr(filewin, " ?");

		snprintf(isbuf, sizeof(isbuf), "%lu", de->size);
		ui_wlpadstr(filewin, isbuf, dir->longest_size + 1);

		strftime(stime, sizeof(stime), "%b %d %H:%M %Y", localtime(&de->mtime));
		wprintw(filewin, " %s ", stime);
	}

	int color = COLOR_PAIR(ui_dirent_color(de));
	if (selected) color |= A_REVERSE;
	wattron(filewin, color);
	waddstr(filewin, de->name);
	wattroff(filewin, color);
	char c = dirent_crepr(de);
	if (c) waddch(filewin, c);
}

static int ui_dirent_color(const dirent_t * de) {
	int color;
	switch (de->type) {
		case DE_FILE:    color = COLOR_FILE;    break;
		case DE_DIR:     color = COLOR_DIR;     break;
		case DE_FIFO:    color = COLOR_FIFO;    break;
		case DE_LINK:    color = COLOR_LINK;    break;
		case DE_BLOCK:   color = COLOR_BLOCK;   break;
		case DE_CHAR:    color = COLOR_CHAR;    break;
		case DE_SOCKET:  color = COLOR_SOCKET;  break;
		case DE_UNKNOWN: color = COLOR_UNKNOWN; break;
		default:         color = WHITE;         break;
	}

	if (de->type == DE_FILE && dirent_isexec(de)) {
		color = COLOR_EXEC;
	}

	return color;
}

void ui_print_dir(const dir_t * dir, size_t cursor, bool longmode) {
	size_t first;
	size_t last;
	ui_get_first_last(dir->len, cursor, &first, &last);

	for (size_t i = first; i < last; i++) {
		size_t pos = i - first;
		ui_print_dirent(&dir->entries[i], pos, cursor == i, longmode, dir);
	}
}

void ui_print_cursor(size_t cursor, size_t total) {
	size_t lines, cols;
	getmaxyx(filewin, lines, cols);
	if (total < lines - 2) wmove(filewin, total + 1, 0);
	else wmove(filewin, lines - 1, 0);

	wprintw(filewin, "%zu/%zu", cursor + 1, total);
}

static void ui_get_first_last(size_t dir_len, size_t cursor, size_t * first, size_t * last) {
	size_t lines, cols;
	getmaxyx(filewin, lines, cols);
	lines -= 2;
	// try to keep cursor in the middle of the window
	if (dir_len < lines) {
		*first = 0;
		*last = dir_len;
	} else {
		size_t off = lines / 2;
		if (cursor < off) {
			*first = 0;
			if (dir_len > lines) *last = lines;
			else *last = dir_len;
		} else {
			*first = cursor - off;
			if (dir_len > cursor + off) *last = cursor + off;
			else *last = dir_len;
		}
	}
}
