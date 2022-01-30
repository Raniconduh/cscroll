#include <stdbool.h>

#include "hash.h"
#include "opts.h"
#include "io.h"


static map * var_map = NULL;


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


void var_init(void) {
	var_map = map_new(4);

	map_insert(var_map, ICONS_VAR, set_icons);
	map_insert(var_map, COLOR_VAR, set_color_f);
	map_insert(var_map, LONG_VAR,  set_long);
}


bool var_set(char * k, void * p) {
	void (*set_func)(void *) = map_index(var_map, k);
	if (!set_func) return false;

	set_func(p);
	return true;
}


void terminate_var(void) {
	map_nuke(var_map);
}
