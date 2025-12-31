#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include "dir.h"
#include "cvector.h"

#define CWDSZ (PATH_MAX + 1)
#define LINKNAMESZ PATH_MAX

static char cwd_buf[CWDSZ];
static char * cwd = NULL;

static void dir_entry(int dirfd, const char * name, dirent_t * dirent);
static char de_crepr(enum de_type de_type);
static void free_dirent(void * p) {
	dirent_t * de = (dirent_t*)p;
	free(de->name);
	free(de->uname);
	free(de->gname);
	free(de->linkname);
}
static size_t ilen(size_t i, int base) {
	if (i == 0) return 1;
	size_t r = 0;
	while (i != 0) {
		r++;
		i /= base;
	}
	return r;
}

int dir_list(const char * path, dir_t * dir) {
	dir->len = 0;
	dir->entries = NULL;
	cvector_init(dir->entries, 1, free_dirent);
	dir->longest_uname = 0;
	dir->longest_gname = 0;
	dir->longest_size = 0;

	struct dirent * de;
	DIR * dp = opendir(path);
	if (!dp) {
		return -errno;
	}

	while ((de = readdir(dp))) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
			continue;
		}

		dirent_t dirent;
		dir_entry(dirfd(dp), de->d_name, &dirent);
		cvector_push_back(dir->entries, dirent);
		dir->len++;

		size_t uname_len = 0;
		size_t gname_len = 0;
		size_t size_len = ilen(dirent.size, 10);
		if (dirent.uname) uname_len = strlen(dirent.uname);
		if (dirent.gname) gname_len = strlen(dirent.gname);
		if (uname_len > dir->longest_uname) dir->longest_uname = uname_len;
		if (gname_len > dir->longest_gname) dir->longest_gname = gname_len;
		if (size_len > dir->longest_size) dir->longest_size = size_len;
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
		// every non string field other than type is left uninitialized
		// strings are initialized to NULL
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

char de_crepr(enum de_type de_type) {
	switch (de_type) {
		case DE_SOCKET: return '=';
		case DE_LINK:   return '@';
		case DE_FILE:   return 0;
		case DE_BLOCK:  return '#';
		case DE_DIR:    return '/';
		case DE_CHAR:   return '#';
		case DE_FIFO:   return '|';
		default:        return '?';
	}
}

char dirent_crepr(const dirent_t * de) {
	char c = de_crepr(de->type);
	if (!c && dirent_isexec(de)) return '*';
	return c;
}

char dirent_creprl(const dirent_t * de) {
	return de_crepr(de->linktype);
}

char dirent_longcrepr(const dirent_t * de) {
	switch (de->type) {
		case DE_SOCKET: return '=';
		case DE_LINK:   return 'l';
		case DE_BLOCK:  return 'b';
		case DE_DIR:    return 'd';
		case DE_CHAR:   return 'c';
		case DE_FIFO:   return '|';
		default:        return '.';
	}
}

bool dirent_isexec(const dirent_t * de) {
	// this is because the condition can be truthy but is not actually a bool
	return (de->mode & (M_USRX | M_GRPX | M_OTHX)) ? true : false;
}

const char * dirent_prettymode(const dirent_t * de) {
	// 1 repr char, 9 mode bits, 1 null byte
	static char s[11];
	char * p = s;

	if (de->type == DE_UNKNOWN) {
		strcpy(s, "??????????");
		return s;
	}

	*p++ = dirent_longcrepr(de);
	*p++ = (de->mode & M_USRR) ? 'r' : '-';
	*p++ = (de->mode & M_USRW) ? 'w' : '-';
	*p++ = (de->mode & M_USRX) ?
	         (de->mode & M_SUID) ? 's' : 'x'
		   : (de->mode & M_SUID) ? 'S' : '-';
	*p++ = (de->mode & M_GRPR) ? 'r' : '-';
	*p++ = (de->mode & M_GRPW) ? 'w' : '-';
	*p++ = (de->mode & M_GRPX) ?
	         (de->mode & M_SGID) ? 's' : 'x'
	       : (de->mode & M_SGID) ? 'S' : '-';
	*p++ = (de->mode & M_OTHR) ? 'r' : '-';
	*p++ = (de->mode & M_OTHW) ? 'w' : '-';
	*p++ = (de->mode & M_OTHX) ?
	         (de->mode & M_STICKY) ? 't' : 'x'
	       : (de->mode & M_STICKY) ? 'T' : '-';

	*p = 0;

	return s;
}

const char * dir_get_cwd(void) {
	if (!cwd) {
		getcwd(cwd_buf, CWDSZ);
		cwd = cwd_buf;
		char * p = strrchr(cwd, '/');
		if (*(p + 1) == 0) *p = 0;
	}
	return cwd;
}

int dir_cd_back(const char * wd) {
	if (!*wd) return -EPERM;
	if (!strcmp(wd, "/")) return 0;

	char * newwd = strdup(wd);
	char * p = strrchr(newwd, '/');
	*p = 0;
	if (*newwd == 0) {
		*newwd = '/';
		*(newwd + 1) = 0;
	}
	int ret = chdir(newwd);
	int terrno = errno;
	if (ret >= 0) strcpy(cwd, newwd);
	free(newwd);

	if (ret < 0) return -terrno;
	return 0;
}

int dir_cd(const char * wd, const char * next) {
	size_t wdlen = strlen(wd);
	size_t nextlen = strlen(next);

	// extra byte for '/' character
	char * newwd = malloc(wdlen + nextlen + 1 + 1);
	if (strcmp(wd, "/") != 0) {
		strcpy(newwd, wd);
		newwd[wdlen] = '/';
		strcpy(newwd + wdlen + 1, next);
	} else {
		newwd[0] = '/';
		strcpy(newwd + 1, next);
	}

	int ret = chdir(newwd);
	int terrno = errno;
	if (ret >= 0) strcpy(cwd, newwd);
	free(newwd);

	if (ret < 0) return -terrno;
	return 0;
}

const char * dir_basename(const char * path) {
	const char * p = strrchr(path, '/');
	if (!p || *(p + 1) == 0) return NULL;
	return p + 1;
}

int dir_search_name(const dir_t * dir, const char * name, size_t * idx) {
	for (size_t i = 0; i < dir->len; i++) {
		if (!strcmp(dir->entries[i].name, name)) {
			*idx = i;
			return 0;
		}
	}

	return -1;
}

int dir_search_regex(const dir_t * dir, const char * regexstr, size_t * idx) {
	regex_t regex;
	int ret = regcomp(&regex, regexstr, REG_EXTENDED);
	if (!!ret) return -REGSEARCH_BAD_REGEX;

	for (size_t i = 0; i < dir->len; i++) {
		dirent_t * de = &dir->entries[i];
		ret = regexec(&regex, de->name, 0, NULL, 0);
		if (!ret) {
			*idx = i;
			regfree(&regex);
			return 0;
		}
	}

	regfree(&regex);
	return -REGSEARCH_NOT_FOUND;
}
