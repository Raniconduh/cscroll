#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <time.h>

#include "ui.h"
#include "dir.h"
#include "config.h"
#include "cvector.h"

#define TITLEWINSTARTY   0
#define TITLEWINLINES    1
#define TITLEWINCOLS     COLS

#define STATUSWINSTARTY  1
#define STATUSWINLINES   1
#define STATUSWINCOLS    COLS

#define FILEWINSTARTY    2
#define FILEWINLINES     (LINES - 3)
#define FILEWINCOLS      COLS

#define INPUTWINSTARTY   (LINES - 1)
#define INPUTWINLINES    1
#define INPUTWINCOLS     COLS

#define INPUTBUFSZ 2048

static WINDOW * titlewin  = NULL;
static WINDOW * statuswin = NULL;
static WINDOW * filewin   = NULL;
static WINDOW * inputwin  = NULL;

static void win_set(WINDOW * win, const char * str, int attrs);
static int ui_dirent_color(const dirent_t * de);
static int ui_link_color(const dirent_t * de);
static void ui_print_dirent(const dirent_t * de, size_t pos, bool selected, const dir_t * dir);
static void ui_wlpadstr(WINDOW * win, const char * s, size_t len);
static void ui_wrpadstr(WINDOW * win, const char * s, size_t len);
static void ui_get_first_last(size_t n_dirents, size_t cursor, size_t * first, size_t * last);

void ui_init(void) {
	setlocale(LC_CTYPE, "");

	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);
	noecho();

	titlewin  = newwin(TITLEWINLINES,  TITLEWINCOLS,  TITLEWINSTARTY,  0);
	statuswin = newwin(STATUSWINLINES, STATUSWINCOLS, STATUSWINSTARTY, 0);
	filewin   = newwin(FILEWINLINES,   FILEWINCOLS,   FILEWINSTARTY,   0);
	inputwin  = newwin(INPUTWINLINES,  INPUTWINCOLS,  INPUTWINSTARTY,  0);

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

void ui_status_info(const char * status) {
	win_set(statuswin, status, 0);
}

void ui_status_error(const char * status) {
	win_set(statuswin, status, COLOR_PAIR(RED));
}

void ui_erase(void) {
	werase(filewin);
	werase(inputwin);
}

void ui_refresh(void) {
	wnoutrefresh(titlewin);
	wnoutrefresh(statuswin);
	wnoutrefresh(filewin);
	wnoutrefresh(inputwin);
	doupdate();
}

static void ui_wlpadstr(WINDOW * win, const char * s, size_t len) {
	size_t slen = strlen(s);

	for (size_t i = slen; i < len; i++) {
		waddch(win, ' ');
	}

	waddstr(win, s);
}

static void ui_wrpadstr(WINDOW * win, const char * s, size_t len) {
	size_t slen = strlen(s);

	waddstr(win, s);

	for (size_t i = slen; i < len; i++) {
		waddch(win, ' ');
	}
}

void ui_print_dirent(const dirent_t * de, size_t pos, bool selected, const dir_t * dir) {
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

	size_t lines, cols;
	getmaxyx(filewin, lines, cols);

	size_t dirlen = dir_len(dir);

	if (config.longmode && (config.longinline || (!config.longinline && selected))) {
		if (config.longinline) wmove(filewin, pos, 0);
		else {
			// place the longmode info right on top of the "cursor" info
			if (dirlen < lines - 3) wmove(filewin, dirlen + 1, 0);
			else wmove(filewin, lines - 2, 0);
		}

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

		if (config.humansize) {
			const char * unit = dirent_size_unit(de);
			snprintf(isbuf, sizeof(isbuf), "%u", de->size_small);
			ui_wlpadstr(filewin, isbuf, dir->longest_size_small + 1);
			ui_wrpadstr(filewin, unit, dir->longest_size_unit);
		} else {
			snprintf(isbuf, sizeof(isbuf), "%zu", de->size);
			ui_wlpadstr(filewin, isbuf, dir->longest_size + 1);
		}

		strftime(stime, sizeof(stime), "%b %d %H:%M %Y", localtime(&de->mtime));
		wprintw(filewin, " %s ", stime);
	}

	if (!config.longmode || (config.longmode && !config.longinline))
		wmove(filewin, pos, 0);

	int color = COLOR_PAIR(ui_dirent_color(de));
	if (selected) color |= A_REVERSE;
	waddch(filewin, ' ');
	wattron(filewin, color);
	waddstr(filewin, de->name);
	wattroff(filewin, color);
	char c = dirent_crepr(de);
	if (c) waddch(filewin, c);

	if (de->type == DE_LINK) {
		waddstr(filewin, " -> ");
		color = COLOR_PAIR(ui_link_color(de));
		if (selected) color |= A_REVERSE;
		wattron(filewin, color);
		waddstr(filewin, de->linkname);
		wattroff(filewin, color);
		char c = dirent_creprl(de);
		if (c) waddch(filewin, c);
	}
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

static int ui_link_color(const dirent_t * de) {
	switch (de->linktype) {
		case DE_FILE:    return COLOR_FILE;
		case DE_DIR:     return COLOR_DIR;
		case DE_FIFO:    return COLOR_FIFO;
		case DE_LINK:    return COLOR_LINK;
		case DE_BLOCK:   return COLOR_BLOCK;
		case DE_CHAR:    return COLOR_CHAR;
		case DE_SOCKET:  return COLOR_SOCKET;
		case DE_UNKNOWN: return COLOR_UNKNOWN;
		default:         return WHITE;
	}
}

void ui_print_dir(const dir_t * dir, size_t cursor) {
	size_t first;
	size_t last;
	size_t len = dir_len(dir);
	cvector(dirent_t) entries = dir_entries(dir);
	ui_get_first_last(len, cursor, &first, &last);

	for (size_t i = first; i < last; i++) {
		size_t pos = i - first;
		ui_print_dirent(&entries[i], pos, cursor == i, dir);
	}
}

void ui_print_cursor(size_t cursor, size_t total) {
	size_t lines, cols;
	getmaxyx(filewin, lines, cols);
	if (config.longmode && !config.longinline) {
		if (total < lines - 3) wmove(filewin, total + 2, 0);
		else wmove(filewin, lines - 1, 0);
	} else {
		if (total < lines - 2) wmove(filewin, total + 1, 0);
		else wmove(filewin, lines - 1, 0);
	}

	if (total > 0) {
		wprintw(filewin, "%zu/%zu", cursor + 1, total);
	} else {
		waddstr(filewin, "0/0");
	}
}

static void ui_get_first_last(size_t dir_len, size_t cursor, size_t * first, size_t * last) {
	size_t lines, cols;
	getmaxyx(filewin, lines, cols);
	lines -= 2;
	if (config.longmode && !config.longinline) lines--;
	// try to keep cursor in the middle of the window
	if (dir_len < lines) {
		*first = 0;
		*last = dir_len;
	} else {
		size_t off_lo = lines / 2;
		size_t off_hi = lines - off_lo;
		if (cursor < off_lo) {
			*first = 0;
			if (dir_len > lines) *last = lines;
			else *last = dir_len;
		} else {
			*first = cursor - off_lo;
			if (dir_len > cursor + off_hi) *last = cursor + off_hi;
			else *last = dir_len;
		}
	}
}

void ui_resize(void) {
	// unless a WINCH handler is specified, ncurses resizes the terminal
	// automatically, so this function just resizes the windows
	wresize(titlewin,  TITLEWINLINES,  TITLEWINCOLS);
	wresize(statuswin, STATUSWINLINES, STATUSWINCOLS);
	wresize(filewin,   FILEWINLINES,   FILEWINCOLS);
	wresize(inputwin,  INPUTWINLINES,  INPUTWINCOLS);

	mvwin(titlewin,  TITLEWINSTARTY,  0);
	mvwin(statuswin, STATUSWINSTARTY, 0);
	mvwin(filewin,   FILEWINSTARTY,   0);
	mvwin(inputwin,  INPUTWINSTARTY,  0);

	ui_refresh();
}

const char * ui_readline(const char * prompt) {
	static char inputbuf[INPUTBUFSZ];

	char * p = inputbuf;
	*p = 0;

	werase(inputwin);
	waddstr(inputwin, prompt);
	wattron(inputwin, A_REVERSE);
	waddch(inputwin, ' ');
	wattroff(inputwin, A_REVERSE);
	wrefresh(inputwin);

	int c;
	while ((c = wgetch(inputwin)) != '\n') {
		if (c == KEY_ESC) {
			curs_set(0);
			return NULL;
		}

		if (c == KEY_DEL || c == KEY_BACKSPACE) {
			if (p != inputbuf) *--p = 0;
			else return NULL;
		} else if (isprint(c)) {
			if (p - inputbuf < INPUTBUFSZ - 1) {
				*p++ = (char)c;
				*p = 0;
			}
		}

		werase(inputwin);
		waddstr(inputwin, prompt);
		waddstr(inputwin, inputbuf);
		wattron(inputwin, A_REVERSE);
		waddch(inputwin, ' ');
		wattroff(inputwin, A_REVERSE);
		wrefresh(inputwin);
	}

	if (p == inputbuf) return NULL;
	return inputbuf;
}
