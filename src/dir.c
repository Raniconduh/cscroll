#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "dir.h"
#include "cvector.h"


static void dir_entry(int dirfd, const char * name, dirent_t * dirent);
static void free_dirent(void * p) {
	free(((dirent_t*)p)->name);
}

int dir_list(const char * path, dir_t * dir) {
	dir->len = 0;
	dir->entries = NULL;
	cvector_init(dir->entries, 1, free_dirent);

	struct dirent * de;
	DIR * dp = opendir(path);
	if (!dp) {
		return -1;
	}

	while ((de = readdir(dp))) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
			continue;
		}

		dirent_t dirent;
		dir_entry(dirfd(dp), de->d_name, &dirent);
		cvector_push_back(dir->entries, dirent);
		dir->len++;
	}

	closedir(dp);

	return 0;
}

void dir_entry(int dirfd, const char * name, dirent_t * dirent) {
	dirent->type = DE_UNKNOWN;
	dirent->size = 0;
	dirent->name = NULL;

	dirent->name = strdup(name);

	struct stat statbuf;
	if (fstatat(dirfd, name, &statbuf, AT_SYMLINK_NOFOLLOW) < 0) {
		return;
	}

	// convert the ugly st_mode to a nice de_type
	switch (statbuf.st_mode & S_IFMT) {
		case S_IFSOCK: dirent->type = DE_SOCKET;  break;
		case S_IFLNK:  dirent->type = DE_LINK;    break;
		case S_IFREG:  dirent->type = DE_FILE;    break;
		case S_IFBLK:  dirent->type = DE_BLOCK;   break;
		case S_IFDIR:  dirent->type = DE_DIR;     break;
		case S_IFCHR:  dirent->type = DE_CHAR;    break;
		case S_IFIFO:  dirent->type = DE_FIFO;    break;
		default:       dirent->type = DE_UNKNOWN; break;
	}

	dirent->size = statbuf.st_size;
}

void dir_free(dir_t * dir) {
	cvector_free(dir->entries);
}
