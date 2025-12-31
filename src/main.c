#include <stdio.h>
#include <stddef.h>

#include "dir.h"

int main(int argc, char ** argv) {
	dir_t dir;
	dir_list(argv[1], &dir);

	for (size_t i = 0; i < dir.len; i++) {
		dirent_t * de = &dir.entries[i];
		char c = dirent_crepr(de);
		if (!c) c = ' ';
		const char * pmode = dirent_prettymode(de);
		printf("%s %s %s %lu %ld %s%c", pmode, de->uname, de->gname, de->size, de->mtime, de->name, c);
		if (de->linkname) {
			c = dirent_creprl(de);
			if (!c) c = ' ';
			printf(" -> %s%c\n", de->linkname, c);
		} else putchar('\n');
	}

	dir_free(&dir);
}
