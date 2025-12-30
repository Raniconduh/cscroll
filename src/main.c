#include <stdio.h>
#include <stddef.h>

#include "dir.h"

int main(int argc, char ** argv) {
	dir_t dir;
	dir_list(argv[1], &dir);

	for (size_t i = 0; i < dir.len; i++) {
		dirent_t * de = &dir.entries[i];
		printf("%s %s %lu %ld %s", de->uname, de->gname, de->size, de->mtime, de->name);
		if (de->linkname) {
			printf(" -> %s\n", de->linkname);
		} else putchar('\n');
	}

	dir_free(&dir);
}
