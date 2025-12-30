#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include "dir.h"
#include "cvector.h"

#define LINKNAMESZ PATH_MAX

static void dir_entry(int dirfd, const char * name, dirent_t * dirent);
static void free_dirent(void * p) {
	dirent_t * de = (dirent_t*)p;
	free(de->name);
	free(de->uname);
	free(de->gname);
	free(de->linkname);
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
	dirent->name = strdup(name);
	dirent->uname = NULL;
	dirent->gname = NULL;
	dirent->linkname = NULL;

	struct stat statbuf;
	if (fstatat(dirfd, name, &statbuf, AT_SYMLINK_NOFOLLOW) < 0) {
		// in the case where stat fails, the file is marked unknown
		// every field other than type and name are left uninitialized
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

	if (S_ISLNK(statbuf.st_mode)) {
		size_t linknamesz;
		if (statbuf.st_size == 0) linknamesz = LINKNAMESZ;
		else linknamesz = statbuf.st_size;
		char * linkname = malloc(linknamesz + 1);
		ssize_t len = readlinkat(dirfd, name, linkname, linknamesz);
		if (len < 0) {
			free(linkname);
			dirent->linkname = NULL;
		} else {
			linkname[len] = 0;
			dirent->linkname = linkname;
		}

		struct stat lstatbuf;
		if (fstatat(dirfd, name, &lstatbuf, 0) < 0) {
			dirent->linktype = DE_UNKNOWN;
		} else switch (lstatbuf.st_mode & S_IFMT) {
			case S_IFSOCK: dirent->linktype = DE_SOCKET;  break;
			case S_IFLNK:  dirent->linktype = DE_LINK;    break;
			case S_IFREG:  dirent->linktype = DE_FILE;    break;
			case S_IFBLK:  dirent->linktype = DE_BLOCK;   break;
			case S_IFDIR:  dirent->linktype = DE_DIR;     break;
			case S_IFCHR:  dirent->linktype = DE_CHAR;    break;
			case S_IFIFO:  dirent->linktype = DE_FIFO;    break;
			default:       dirent->linktype = DE_UNKNOWN; break;
		}
	}

	dirent->size = statbuf.st_size;
	dirent->mode = statbuf.st_mode & 07777;
	dirent->mtime = statbuf.st_mtime;

	struct passwd * pw = getpwuid(statbuf.st_uid);
	if (pw) dirent->uname = strdup(pw->pw_name);

	struct group * gr = getgrgid(statbuf.st_gid);
	if (gr) dirent->gname = strdup(gr->gr_name);
}

void dir_free(dir_t * dir) {
	cvector_free(dir->entries);
}
