#include <stdbool.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "hash.h"
#include "opts.h"
#include "main.h"
#include "dir.h"
#include "io.h"


static map * var_map = NULL;


// must be 6 characters long
static uint32_t hextorgb(char * hex) {
	char * p = hex;
	if (*p == '#') p++;

	char htod[] = {
		['A'] = 10, ['B'] = 11, ['C'] = 12,
		['D'] = 13, ['E'] = 14, ['F'] = 15
	};

	uint16_t r, g, b;
	uint32_t dec = 0;
	int cpow = strlen(p) - 1;

	for (; *p; p++) {
		char c = toupper(*p);
		if (c >= 'A' && c <= 'F') c = htod[(int)c];
		else if (c >= '0' && c <= '9') c -= '0';
		else return COLOR_DEFAULT;
		dec += c * pow(16, cpow);
		cpow--;
	}

	// convert to 10 bit color over 8 bit color
	r = (float)((dec >> 16) & 0xFF) / 255.0 * 1000;
	g = (float)((dec >> 8) & 0xFF) / 255.0 * 1000;
	b = (float)(dec & 0xFF) / 255.0 * 1000;

	return RGB(r, g, b);

	return 0;
}


// void * p -> bool p
static void set_icons(void * p) {
	show_icons = *(bool*)p;
}


static void set_color_f(void * p) {
	color = *(bool*)p;
	set_color();
}


static void set_long(void * p) {
	p_long = *(bool*)p;
}


static void set_dots(void * p) {
	show_dot_files = *(bool*)p;

	free_dir_entries();
	list_dir(cwd);
	cursor = 1;
	first_f = 0;
	last_f = LAST_F;
}


static void set_dir(void * p) {
	dir_color = hextorgb((char*)p);
	set_color();
}


static void set_reg(void * p) {
	reg_color = hextorgb((char*)p);
	set_color();
}


static void set_fifo(void * p) {
	fifo_color = hextorgb((char*)p);
	set_color();
}


static void set_link(void * p) {
	link_color = hextorgb((char*)p);
	set_color();
}


static void set_block(void * p) {
	blk_color = hextorgb((char*)p);
	set_color();
}


static void set_sock(void * p) {
	sock_color = hextorgb((char*)p);
	set_color();
}


static void set_unknown(void * p) {
	unknown_color = hextorgb((char*)p);
	set_color();
}


static void set_exec(void * p) {
	exec_color = hextorgb((char*)p);
	set_color();
}


static void set_media(void * p) {
	media_color = hextorgb((char*)p);
	set_color();
}


static void set_archive(void * p) {
	archive_color = hextorgb((char*)p);
	set_color();
}


static void set_opener(void * p) {
	char * s = (char*)p;
	if (s && *s) {
		size_t l = strlen(s);
		if (l < opener.nlen) {
			opener.nlen = l;
			opener.fpath = realloc(opener.fpath, l + 1);
			strcpy(opener.fpath, s);
		} else if (l == opener.nlen) {
			strcpy(opener.fpath, s);
		} else { /* l > nlen */
			opener.nlen = l;
			opener.fpath = realloc(opener.fpath, l + 1);
			strcpy(opener.fpath, s);
		}
	} else {
		if (opener.fpath) free(opener.fpath);
		opener.fpath = NULL;
		opener.nlen = 0;
	}
}


void var_init(void) {
	var_map = map_new(15);

	map_insert(var_map, ICONS_VAR, set_icons);
	map_insert(var_map, COLOR_VAR, set_color_f);
	map_insert(var_map, LONG_VAR,  set_long);
	map_insert(var_map, DOTS_VAR, set_dots);

	map_insert(var_map, DIR_COLOR_VAR, set_dir);
	map_insert(var_map, REG_COLOR_VAR, set_reg);
	map_insert(var_map, FIFO_COLOR_VAR, set_fifo);
	map_insert(var_map, LINK_COLOR_VAR, set_link);
	map_insert(var_map, BLK_COLOR_VAR, set_block);
	map_insert(var_map, SOCK_COLOR_VAR, set_sock);
	map_insert(var_map, UNKNOWN_COLOR_VAR, set_unknown);
	map_insert(var_map, EXEC_COLOR_VAR, set_exec);
	map_insert(var_map, MEDIA_COLOR_VAR, set_media);
	map_insert(var_map, ARCHIVE_COLOR_VAR, set_archive);

	map_insert(var_map, OPENER_VAR, set_opener);
}


bool var_set(char * k, void * p) {
	void (*set_func)(void *) = map_index(var_map, k);
	if (!set_func) return false;

	set_func(p);
	return true;
}


void terminate_var(void) {
	map_nuke(var_map);
	free(opener.fpath);
}
