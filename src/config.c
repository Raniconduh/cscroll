#include <stdbool.h>

#include "config.h"

config_t config = {
	.longmode   = true,
	.longinline = true,
	.dots       = true,
	.humansize  = true,

	.dir_sortby    = DIR_SORTBY_NAME,
	.dir_sort      = DIR_SORT_INCREASING,
	.dir_sort_dirs = DIR_SORT_DIRS_FIRST,
};
