#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "info.h"
#include "io.h"


struct info_node {
	enum info_t type;
	char * msg;
	time_t start;
	bool disp;
};


static struct {
	WINDOW * w;
	struct info_node ** i;
	size_t n;
} info_buffer = {NULL, NULL, 0};


static int get_info_color(struct info_node * n) {
	switch (n->type) {
		default:
		case INFO_INFO: return COLOR_PAIR(WHITE);
		case INFO_WARN: return COLOR_PAIR(YELLOW) | A_REVERSE;
		case INFO_ERR:  return COLOR_PAIR(RED) | A_REVERSE;
	}
}


void info_init(void) {
	info_buffer.w = newwin(1, COLS, LINES - 1, 0);
	wclear(info_buffer.w);
	refresh();
}


void display_info(enum info_t type, char * fmt, ...) {
	if (!info_buffer.i) info_buffer.i = malloc(sizeof(struct info_node*) * 32);

	// realloc every 32
	if ((info_buffer.n + 1) % 32 == 0) {
		info_buffer.i = realloc(info_buffer.i,
				(info_buffer.n + 32) * sizeof(struct info_node*));
	}

	size_t n = info_buffer.n;
	info_buffer.i[n] = malloc(sizeof(struct info_node));
	info_buffer.i[n]->type = type;

	va_list vlist;
	va_start(vlist, fmt);
	size_t nl = vsnprintf(NULL, 0, fmt, vlist);
	va_end(vlist);

	info_buffer.i[n]->msg = malloc(nl + 1);

	va_start(vlist, fmt);
	vsprintf(info_buffer.i[n]->msg, fmt, vlist);
	va_end(vlist);


	info_buffer.i[n]->disp = true;
	info_buffer.i[n]->start = time(NULL);

	info_buffer.n++;
	refresh_info();
}


void refresh_info(void) {
	size_t n = info_buffer.n - 1;
	if (info_buffer.n == 0 || !info_buffer.w || !info_buffer.i[n]->disp) return;

	if (time(NULL) - info_buffer.i[n]->start >= INFO_TIMEOUT) {
		info_buffer.i[n]->disp = false;
		return;
	}

	int cp = get_info_color(info_buffer.i[n]);
	werase(info_buffer.w);
	waddstr(info_buffer.w, "(:i) ");
	wattron(info_buffer.w, cp);
	waddstr(info_buffer.w, info_buffer.i[n]->msg);
	wattroff(info_buffer.w, cp);
	wrefresh(info_buffer.w);
}


void page_info(void) {
	if (info_buffer.n == 0 || !info_buffer.w) return;

	clear();

	size_t first_info = 0;
	size_t last_info = info_buffer.n - 1;
	if (last_info >= (unsigned)LINES - 1) last_info = LINES - 1;

	bool done = false;

	while (!done) {
		erase();

		for (size_t i = first_info; i <= last_info; i++) {
			int cp = get_info_color(info_buffer.i[i]);
			attron(cp);
			addstr(info_buffer.i[i]->msg);
			attroff(cp);
			addch('\n');
		}

		printw("LINES %lu-%lu/%lu, q to close\n",
				first_info + 1, last_info + 1, info_buffer.n);

		refresh();

		int c = getch();
		switch (c) {
			case UP_KEYS:
				if (first_info > 0) {
					first_info--;
					last_info--;
				}
				break;
			case DOWN_KEYS:
				if (last_info < info_buffer.n - 1) {
					first_info++;
					last_info++;
				}
				break;
			case KEY_RESIZE:
				// jump back to the top, it is fine.
				first_info = 0;
				last_info = info_buffer.n - 1;
				if (last_info >= (unsigned)LINES - 1) last_info = LINES - 1;
				// make sure the file buffer is resized too
				resize_fbuf();
				break;
			case CTRL_C:
			case 'q':
				done = true;
				break;
		}
	}
}
