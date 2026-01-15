#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <strings.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <regex.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <ftw.h>


#include "dir.h"
#include "config.h"
#include "cvector.h"
#include "hashmap.h"

// S_ISVTX requires _XOPEN_SOURCE >= 500
// I guess sticky bit isn't POSIX
#ifndef S_ISVTX
#define S_ISVTX 0
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef UNUSED
#error UNUSED is already defined
#undef UNUSED
#endif
#ifndef UNUSED
#	if __GNUC__ || defined(__clang__)
#		define UNUSED __attribute__((unused))
#	elif __STDC_VERSION__ >= 202311L
#		define UNUSED [[maybe_unused]]
#	else
#		define UNUSED
#	endif
#endif /* UNUSED */

#define CWDSZ (PATH_MAX + 1)
#define LINKNAMESZ PATH_MAX

static char cwd_buf[CWDSZ];
static char * cwd = NULL;
// marks is a map of sets
// keys are directories and values are sets of marked files in that directory
static hashmap * marks = NULL;
static size_t n_marks = 0;

static void dir_entry(int dirfd, const char * name, dirent_t * dirent, const hashmap * dirmarks);
static void dir_fill_nodots(dir_t * dir);
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
static void marks_destroyer(void * p) {
	hashmap_destroy((hashmap*)p);
}

void dir_init(void) {
	// read cwd
	getcwd(cwd_buf, CWDSZ);
	cwd = cwd_buf;
	char * p = strrchr(cwd, '/');
	if (*(p + 1) == 0) *p = 0;

	marks = hashmap_new(marks_destroyer);
}

void dir_deinit(void) {
	hashmap_destroy(marks);
}

int dir_list(const char * path, dir_t * dir) {
	dir->len = 0;
	dir->entries = NULL;
	cvector_init(dir->entries, 1, free_dirent);
	dir->nodots_len = 0;
	dir->nodots = NULL;
	dir->longest_uname = 0;
	dir->longest_gname = 0;
	dir->longest_size = 0;
	dir->longest_size_small = 0;
	dir->longest_size_unit = 0;

	struct dirent * de;
	DIR * dp = opendir(path);
	if (!dp) {
		return -errno;
	}

	hashmap * dirmarks = hashmap_get(marks, path);

	while ((de = readdir(dp))) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
			continue;
		}

		dirent_t dirent;
		dir_entry(dirfd(dp), de->d_name, &dirent, dirmarks);
		cvector_push_back(dir->entries, dirent);
		dir->len++;

		size_t uname_len = 0;
		size_t gname_len = 0;
		size_t size_len = ilen(dirent.size, 10);
		size_t size_small_len = ilen(dirent.size_small, 10);
		size_t size_unit_len = (dirent.size_unit == SIZE_B) ? 1 : 2;
		if (dirent.uname) uname_len = strlen(dirent.uname);
		if (dirent.gname) gname_len = strlen(dirent.gname);
		if (uname_len > dir->longest_uname) dir->longest_uname = uname_len;
		if (gname_len > dir->longest_gname) dir->longest_gname = gname_len;
		if (size_len > dir->longest_size) dir->longest_size = size_len;
		if (size_small_len > dir->longest_size_small)
			dir->longest_size_small = size_small_len;
		if (size_unit_len > dir->longest_size_unit)
			dir->longest_size_unit = size_unit_len;
	}

	closedir(dp);
	dir_fill_nodots(dir);
	dir_sort(dir);
	return 0;
}

static void dir_fill_nodots(dir_t * dir) {
	cvector_clear(dir->nodots);
	dir->nodots_len = 0;

	for (size_t i = 0; i < dir->len; i++) {
		dirent_t * dirent = &dir->entries[i];
		if (*dirent->name != '.') {
			cvector_push_back(dir->nodots, dirent);
			dir->nodots_len++;
		}
	}
}

void dir_entry(int dirfd, const char * name, dirent_t * dirent, const hashmap * dirmarks) {
	memset(dirent, 0, sizeof(dirent_t));
	dirent->name = strdup(name);
	dirent->type = DE_UNKNOWN;

	struct stat statbuf;
	if (fstatat(dirfd, name, &statbuf, AT_SYMLINK_NOFOLLOW) < 0) {
		// in the case where stat fails, the file is marked unknown
		return;
	}

	if (dirmarks && hashmap_get(dirmarks, name)) dirent->marked = true;

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

	size_t size = statbuf.st_size;
	enum size_unit size_unit = SIZE_B;
	for (;size >= 1024;) {
		size /= 1024;
		size_unit++;
		if (size_unit == SIZE_EB) break;
	}

	dirent->size = statbuf.st_size;
	dirent->size_small = size;
	dirent->size_unit = size_unit;

	dirent->mode = statbuf.st_mode & 07777;
	dirent->mtime = statbuf.st_mtime;

	struct passwd * pw = getpwuid(statbuf.st_uid);
	if (pw) dirent->uname = strdup(pw->pw_name);

	struct group * gr = getgrgid(statbuf.st_gid);
	if (gr) dirent->gname = strdup(gr->gr_name);
}

void dir_free(dir_t * dir) {
	cvector_free(dir->entries);
	dir->len = 0;
	cvector_free(dir->nodots);
	dir->nodots_len = 0;
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
	size_t dirlen = dir_len(dir);

	for (size_t i = 0; i < dirlen; i++) {
		dirent_t * de = dir_get_entry(dir, i);
		if (!strcmp(de->name, name)) {
			*idx = i;
			return 0;
		}
	}

	return -1;
}

int dir_search_regex(const dir_t * dir, const char * regexstr, size_t * idx) {
	size_t dirlen = dir_len(dir);

	regex_t regex;
	int ret = regcomp(&regex, regexstr, REG_EXTENDED);
	if (!!ret) return -REGSEARCH_BAD_REGEX;

	for (size_t i = 0; i < dirlen; i++) {
		dirent_t * de = dir_get_entry(dir, i);
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

size_t dir_len(const dir_t * dir) {
	if (config.dots) return dir->len;
	else return dir->nodots_len;
}

dirent_t * dir_get_entry(const dir_t * dir, size_t cursor) {
	if (dir_len(dir) == 0) return NULL;

	if (config.dots) return &dir->entries[cursor];
	else return dir->nodots[cursor];
}

static int dir_sort_cmp(const void * inpa, const void * inpb) {
	const dirent_t * a = (const dirent_t*)inpa;
	const dirent_t * b = (const dirent_t*)inpb;

	if (config.dir_sort_dirs != DIR_SORT_DIRS_UNSORTED
			&& ((a->type == DE_DIR) != (b->type == DE_DIR))) {
		if (config.dir_sort_dirs == DIR_SORT_DIRS_FIRST)
			return (a->type == DE_DIR) ? -1 : 1;
		if (config.dir_sort_dirs == DIR_SORT_DIRS_LAST)
			return (b->type == DE_DIR) ? -1 : 1;
	}

	if (config.dir_sort == DIR_SORT_DECREASING) {
		const dirent_t * tmp = a;
		a = b;
		b = tmp;
	}

	switch (config.dir_sortby) {
		case DIR_SORTBY_NAME:
			return strcasecmp(a->name, b->name);
		case DIR_SORTBY_TIME:
			return (a->mtime < b->mtime) ? -1 : 1;
		case DIR_SORTBY_SIZE:
			return (a->size < b->size) ? -1 : 1;
		default: return 0;
	}
}


void dir_sort(dir_t * dir) {
	if (config.dir_sort == DIR_SORT_UNSORTED) return;
	if (dir->entries && dir->len > 0)
		qsort(dir->entries, dir->len, sizeof(dir->entries[0]), dir_sort_cmp);
	dir_fill_nodots(dir);
}

const char * dirent_size_unit(const dirent_t * de) {
	static const char * units[] = {
		[SIZE_B]  = "B",  [SIZE_KB] = "kB", [SIZE_MB] = "MB",
		[SIZE_GB] = "GB", [SIZE_TB] = "TB", [SIZE_EB] = "EB",
	};
	static const char * nounit = "";

	const char * ret = units[de->size_unit];
	if (!ret) return nounit;
	return ret;
}

static int dir_internal_ftw_(
	int dfd,
	int (*cb)(
		int dirfd,
		const char * path,
		const struct stat * sb,
		void * arg
	),
	void * arg,
	bool postorder,

	dev_t rootdev
) {
	int retval = 0;

	DIR * dp = fdopendir(dfd);
	if (!dp) return 0;
	dfd = dirfd(dp);

	struct dirent * de;
	while ((de = readdir(dp)) != NULL) {
		int ret;

		char * name = de->d_name;
		if (!strcmp(name, ".") || !strcmp(name, "..")) continue;

		struct stat statbuf;
		ret = fstatat(dfd, name, &statbuf, AT_SYMLINK_NOFOLLOW);
		struct stat * statbufp;
		if (ret < 0) statbufp = NULL;
		else statbufp = &statbuf;

		if (statbuf.st_dev != rootdev) continue;

		// do an inorder traversal
		if (!postorder) {
			ret = cb(dfd, name, statbufp, arg);
			// if cb returns a negative value, stop the walk immediately
			if (ret < 0) {
				retval = ret;
				goto done;
			}
		}

		if (S_ISDIR(statbuf.st_mode)) {
			int nextfd = openat(dfd, name, O_RDONLY | O_DIRECTORY);
			ret = dir_internal_ftw_(nextfd, cb, arg, postorder, rootdev);
			if (ret < 0) {
				retval = ret;
				goto done;
			}
		}

		// do a postorder traversal
		if (postorder) {
			ret = cb(dfd, name, statbufp, arg);
			if (ret < 0) {
				retval = ret;
				goto done;
			}
		}
	}

done:
	closedir(dp);
	return retval;
}

static int dir_internal_ftw(
	const char * path,
	int (*cb)(
		int dirfd,
		const char * path,
		const struct stat * sb,
		void * arg
	),
	void * arg,
	bool postorder
) {
	DIR * d = opendir(path);
	if (!d) return -1;

	struct stat statbuf;
	// I don't think it can error here
	if (lstat(path, &statbuf) < 0) {
		closedir(d);
		return -1;
	}

	dev_t rootdev = statbuf.st_dev;

	int ret = dir_internal_ftw_(dup(dirfd(d)), cb, arg, postorder, rootdev);
	closedir(d);
	return ret;
}

static int dirent_subfiles_ftw_cb(
	UNUSED int dirfd, UNUSED const char * path,
	UNUSED const struct stat * sb, void * arg
) {
	(*(size_t*)arg)++;
	return 0;
}

static int dirent_delete_ftw_cb(
	int dirfd, const char * path, const struct stat * sb,
	UNUSED void * arg
) {
	if (!sb) return 0;

	int flags = 0;
	if (S_ISDIR(sb->st_mode)) flags = AT_REMOVEDIR;

	unlinkat(dirfd, path, flags);
	return 0;
}

size_t dirent_subfiles(const dirent_t * de) {
	if (de->type != DE_DIR) return 0;

	size_t count = 0;
	dir_internal_ftw(de->name, dirent_subfiles_ftw_cb, &count, false);
	return count;
}

int dirent_delete(const dirent_t * de) {
	if (de->type == DE_DIR) {
		int ret = dir_internal_ftw(de->name, dirent_delete_ftw_cb, NULL, true);
		if (ret >= 0) {
			if (rmdir(de->name) < 0) return -errno;
			return 0;
		}
		return ret;
	} else {
		if (unlink(de->name) < 0) return -errno;
		return 0;
	}
}

int dirent_open(const dirent_t * de) {
	pid_t pid = fork();
	if (pid < 0) return -errno;

	if (!pid) {
		// detach everything from this terminal basically
		int fd = open("/dev/null", O_RDWR);
		if (fd < 0) {
			close(STDOUT_FILENO);
			close(STDIN_FILENO);
		} else {
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDIN_FILENO);
		}
		execvp(config.opener, (char*[]){config.opener, de->name, NULL});
		if (fd >= 0) close(fd);
		exit(0);
	}

	waitpid(pid, NULL, 0);
	return 0;
}

static void dir_normalize_path(char * path) {
	if (!path || !*path) return;

	char * base = path + 1;
	// remove duplicate separators
	for (char * p = base; *p; p++) {
		// a + a'b = a + b
		if (*p != '/' || *(base - 1) != '/')
			*base++ = *p;
	}
	*base = 0;

	// remove trailing separators
	while (base > path + 1 && *(base - 1) == '/') {
		*(base - 1) = 0;
		base--;
	}
}

const char * dir_get_home(void) {
	static char home_buf[PATH_MAX+1];

	uid_t uid = getuid();
	struct passwd * pw = getpwuid(uid);
	if (!pw) {
		const char * envhome = getenv("HOME");
		if (!envhome) return NULL;
		strncpy(home_buf, envhome, PATH_MAX);
		home_buf[PATH_MAX] = 0;

		if (!home_buf[0]) return NULL; // idk but HOME should be absolute
		dir_normalize_path(home_buf);
		return home_buf;
	}
	return pw->pw_dir;
}

size_t dir_get_total_marked(void) {
	return n_marks;
}

static size_t dir_get_total_marked_(void) {
	size_t n_marks = 0;

	hashmap_walk_state marksws = {0};
	while (hashmap_walk(marks, &marksws)) {
		hashmap * dirmarks = marksws.val;
		hashmap_walk_state dirmarksws = {0};
		while (hashmap_walk(dirmarks, &dirmarksws))
			n_marks++;
	}

	return n_marks;
}

int dirent_togglemark(dirent_t * de) {
	if (de->type == DE_UNKNOWN) return -TOGGLEMARK_DIRENT_UNKNOWN;

	if (de->marked) {
		hashmap * dirmarks = hashmap_get(marks, cwd);
		de->marked = false;
		hashmap_remove(dirmarks, de->name);
		if (dirmarks->len == 0) hashmap_remove(marks, cwd);
		n_marks--;
	} else {
		// do not allow marking if an ancestor is already marked
		bool bad = false;
		char * ancestor = strdup(cwd);
		do {
			char * sep = strrchr(ancestor, '/');
			*sep = 0;
			char * base = sep + 1;

			hashmap * ancestor_marks;
			if (sep == ancestor) ancestor_marks = hashmap_get(marks, "/");
			else ancestor_marks = hashmap_get(marks, ancestor);

			if (!ancestor_marks) continue;
			if (hashmap_get(ancestor_marks, base)) {
				bad = true;
				break;
			}
		} while (*ancestor);
		free(ancestor);
		if (bad) return -TOGGLEMARK_PARENT_MARKED;

		de->marked = true;
		hashmap * dirmarks = hashmap_get(marks, cwd);
		if (!dirmarks) {
			dirmarks = hashmap_new(NULL);
			hashmap_insert(marks, cwd, dirmarks);
		}
		hashmap_insert(dirmarks, de->name, (void*)true);

		// unmark all children if marking a directory
		if (de->type == DE_DIR) {
			const char * fmt;
			if (!strcmp(cwd, "/")) fmt = "%s%s";
			else fmt = "%s/%s";

			size_t dirpathlen = snprintf(NULL, 0, fmt, cwd, de->name);
			// extra byte for trailing slash
			char * dirpath = malloc(dirpathlen + 1 + 1);
			sprintf(dirpath, fmt, cwd, de->name);

			// check any direct children
			if (hashmap_get(marks, dirpath)) hashmap_remove(marks, dirpath);

			// check any subchildren
			strcat(dirpath, "/");
			dirpathlen++;
			bool deleted;
			do {
				deleted = false;

				// this is a horrible solution
				hashmap_walk_state ws = {0};
				while (hashmap_walk(marks, &ws)) {
					if (!strncmp(ws.key, dirpath, dirpathlen)) {
						hashmap_remove(marks, ws.key);
						deleted = true;
						break;
					}
				}
			} while (deleted);

			free(dirpath);
		}

		n_marks = dir_get_total_marked_();
	}

	return 0;
}

int dir_paste_marks(const char * cwd, size_t * total_pastes) {
	*total_pastes = 0;

	DIR * pastedir = opendir(cwd);
	if (!pastedir) return -errno;
	int pastefd = dirfd(pastedir);

	int retval = 0;

	hashmap_walk_state marksws = {0};
	while (hashmap_walk(marks, &marksws)) {
		const char * dirpath = marksws.key;
		hashmap * dirmarks = marksws.val;

		DIR * sourcedir = opendir(dirpath);
		if (!sourcedir) continue;
		int sourcefd = dirfd(sourcedir);

		hashmap_walk_state dirmarksws = {0};
		while (hashmap_walk(dirmarks, &dirmarksws)) {
			const char * fname = dirmarksws.key;
			// make sure the new file name doesn't already exist
			struct stat statbuf;
			int ret = fstatat(pastefd, fname, &statbuf, AT_SYMLINK_NOFOLLOW);
			if (ret >= 0) continue;
			ret = renameat(sourcefd, fname, pastefd, fname);
			if (ret < 0) continue;
			(*total_pastes)++;
		}

		closedir(sourcedir);
	}

	closedir(pastedir);
	hashmap_destroy(marks);
	marks = hashmap_new(marks_destroyer);
	n_marks = 0;
	return retval;
}

size_t dir_marked_subfiles(void) {
	size_t total = 0;

	hashmap_walk_state marksws = {0};
	while (hashmap_walk(marks, &marksws)) {
		const char * dirpath = marksws.key;
		hashmap * dirmarks = marksws.val;

		DIR * basedir = opendir(dirpath);
		if (!basedir) continue;
		int basedirfd = dirfd(basedir);

		hashmap_walk_state dirmarksws = {0};
		while (hashmap_walk(dirmarks, &dirmarksws)) {
			const char * fname = dirmarksws.key;

			struct stat statbuf;
			int ret = fstatat(basedirfd, fname, &statbuf, AT_SYMLINK_NOFOLLOW);
			if (ret < 0) continue;
			if (!S_ISDIR(statbuf.st_mode)) continue;

			char * subpath = malloc(strlen(dirpath) + 1 + strlen(fname) + 1);
			sprintf(subpath, "%s/%s", dirpath, fname);

			size_t subfiles = 0;
			dir_internal_ftw(subpath, dirent_subfiles_ftw_cb, &subfiles, false);
			free(subpath);
			total += subfiles;
		}

		closedir(basedir);
	}

	return total;
}

int dir_marked_delete(void) {
	int retval = 0;

	hashmap_walk_state marksws = {0};
	while (hashmap_walk(marks, &marksws)) {
		const char * dirpath = marksws.key;
		hashmap * dirmarks = marksws.val;

		hashmap_walk_state dirmarksws = {0};
		while (hashmap_walk(dirmarks, &dirmarksws)) {
			const char * fname = dirmarksws.key;

			char * subpath = malloc(strlen(dirpath) + 1 + strlen(fname) + 1);
			sprintf(subpath, "%s/%s", dirpath, fname);

			// subpath is absolute so dirfd isn't used
			dirent_t de;
			dir_entry(-1, subpath, &de, NULL);
			free(subpath);

			int ret = dirent_delete(&de);
			if (ret < 0) retval = ret;
			free_dirent(&de);
		}
	}

	hashmap_destroy(marks);
	marks = hashmap_new(marks_destroyer);
	n_marks = 0;

	return retval;
}
