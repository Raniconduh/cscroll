#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

enum dir_sort_dirs {
	DIR_SORT_DIRS_FIRST,
	DIR_SORT_DIRS_LAST,
	DIR_SORT_DIRS_UNSORTED,
};

enum dir_sortby {
	DIR_SORTBY_NAME,
	DIR_SORTBY_TIME,
	DIR_SORTBY_SIZE,
};

enum dir_sort {
	DIR_SORT_INCREASING,
	DIR_SORT_DECREASING,
	DIR_SORT_UNSORTED,
};

typedef struct {
	bool longmode;
	bool longinline;
	bool dots;
	bool humansize;
	enum dir_sort_dirs dir_sort_dirs;
	enum dir_sortby dir_sortby;
	enum dir_sort dir_sort;
} config_t;

extern config_t config;

#endif /* CONFIG_H */
