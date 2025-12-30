#include <stdio.h>
#include <stddef.h>

#include "dir.h"

int main(int argc, char ** argv) {
	dir_t dir;
	dir_list(argv[1], &dir);

	for (size_t i = 0; i < dir.len; i++) {
		printf("%lu %s\n", dir.entries[i].size, dir.entries[i].name);
	}

	dir_free(&dir);
}
