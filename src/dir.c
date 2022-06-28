#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "type.h"
#include "opts.h"
#include "dir.h"
#include "io.h"


char * cwd = NULL;

size_t n_dir_entries = 0;
size_t dir_longest_owner = 0;
size_t dir_longest_group = 0;
struct dir_entry_t ** dir_entries = NULL;

bool permission_denied = false;


static int cmp(const void * a, const void * b) {
	const struct dir_entry_t * c = *(const struct dir_entry_t **)a;
	const struct dir_entry_t * d = *(const struct dir_entry_t **)b;

	// sort directories first
	if (c->file_type == FILE_DIR && d->file_type != FILE_DIR)
		return -1;
	else if (d->file_type == FILE_DIR && c->file_type != FILE_DIR)
		return 1;
	else
		return strcmp(c->name, d->name);
}

static int acmp(const void * a, const void * b) {
	return strcmp((*(const struct dir_entry_t**)a)->name,
			(*(const struct dir_entry_t**)b)->name);
}


int list_dir(char * dir_path) {
	struct dirent * d_entry;
	DIR * dir = opendir(dir_path);

	n_dir_entries = 0;

	if (!dir) {
		if (errno == EACCES)
			permission_denied = true;
		closedir(dir);
		return 1;
	}
	permission_denied = false;

	while ((d_entry = readdir(dir))) {
		struct dir_entry_t * dir_entry = malloc(sizeof(struct dir_entry_t) + 12);

		char * d_name = d_entry->d_name;
		size_t d_name_len = strlen(d_name);

		if (!strcmp(d_name, ".") || !strcmp(d_name, "..")) {
			free(dir_entry);
			continue;
		}
		else if (!show_dot_files && d_name[0] == '.') {
			free(dir_entry);
			continue;
		}

		struct stat * buf = malloc(sizeof(struct stat));
		char * tmp_path = malloc(d_name_len + strlen(dir_path) + 2);
		sprintf(tmp_path, "%s/%s", dir_path, d_name);
		lstat(tmp_path, buf);

		dir_entries = realloc(dir_entries, sizeof(struct dir_entry_t) * (n_dir_entries + 1));
		dir_entry->name = malloc(d_name_len + 1);
		
		strcpy(dir_entry->name, d_name);

		dir_entry->file_type = FILE_UNKNOWN;
		switch(buf->st_mode & S_IFMT) {
			case S_IFBLK:
			case S_IFCHR:
				dir_entry->file_type = FILE_BLK;
				break;
			case S_IFSOCK:
				dir_entry->file_type = FILE_SOCK;
				break;
			case S_IFDIR:
				dir_entry->file_type = FILE_DIR;
				break;
			case S_IFLNK:
				dir_entry->file_type = FILE_LINK;
				struct stat * buf2 = malloc(sizeof(struct stat));
				stat(tmp_path, buf2);
				if ((buf2->st_mode & S_IFMT) == S_IFDIR)
					dir_entry->under_link = FILE_DIR;
				else dir_entry->under_link = FILE_UNKNOWN;
				free(buf2);
				break;
			case S_IFIFO:
				dir_entry->file_type = FILE_FIFO;
				break;
			case S_IFREG:
				dir_entry->file_type = FILE_REG;
				break;
			default:
				dir_entry->file_type = FILE_UNKNOWN;
				break;
		}

		dir_entry->mode = 0;

		// other mode
		if (buf->st_mode & S_IROTH)
			dir_entry->mode |= M_READ;
		if (buf->st_mode & S_IWOTH)
			dir_entry->mode |= M_WRITE;
		if (buf->st_mode & S_IXOTH)
			dir_entry->mode |= M_EXEC;

		// group mode
		if (buf->st_mode & S_IRGRP)
			dir_entry->mode |= PGROUP(M_READ);
		if (buf->st_mode & S_IWGRP)
			dir_entry->mode |= PGROUP(M_WRITE);
		if (buf->st_mode & S_IXGRP)
			dir_entry->mode |= PGROUP(M_EXEC);

		// owner mode
		if (buf->st_mode & S_IRUSR)
			dir_entry->mode |= POWNER(M_READ);
		if (buf->st_mode & S_IWUSR)
			dir_entry->mode |= POWNER(M_WRITE);
		if (buf->st_mode & S_IXUSR)
			dir_entry->mode |= POWNER(M_EXEC);

		if (buf->st_mode & S_ISUID)
			dir_entry->mode |= M_SUID;

#if defined(__APPLE__) || defined(__MACH__)
		dir_entry->mtime = buf->st_mtime;	
#else
		dir_entry->mtime = buf->st_mtim.tv_sec;
#endif
		size_t n;
		dir_entry->owner = getpwuid(buf->st_uid)->pw_name;
		if ((n = strlen(dir_entry->owner)) > dir_longest_owner)
			dir_longest_owner = n;
		dir_entry->group = getgrgid(buf->st_gid)->gr_name;
		if ((n = strlen(dir_entry->group)) > dir_longest_group)
			dir_longest_group = n;

		dir_entry->size = buf->st_size;
		dir_entry->u_size = 0;

		for (int i = 0; i <= PB; i++) {
			if (dir_entry->size < 1000) break;
			dir_entry->size /= 1000;
			dir_entry->u_size++;
		}

		free(tmp_path);
		free(buf);

		// figure out file 'mime' type
		dir_entry->m_type = get_mime(dir_entry->name);

		dir_entry->marked = false;

		dir_entries[n_dir_entries] = dir_entry;
		n_dir_entries++;
	}

	closedir(dir);

	qsort(dir_entries, n_dir_entries, sizeof(struct dir_entry_t*), cmp);
	size_t d_end = 0;
	for (size_t i = 0; i < n_dir_entries; i++) {
		if (dir_entries[i]->file_type != FILE_DIR) {
			d_end = i;
			break;
		}
	}
	qsort(dir_entries, d_end, sizeof(struct dir_entry_t*), acmp);
	qsort(dir_entries + d_end, n_dir_entries - d_end,
			sizeof(struct dir_entry_t*), acmp);


	return 0;
}


void free_dir_entries(void) {
	for (size_t i = 0; i < n_dir_entries; i++) {
		free(dir_entries[i]->name);
		free(dir_entries[i]);
	}
	
	n_dir_entries = 0;
}


void cd_back(void) {
	char * p = strrchr(cwd, '/');
	*p = '\0';
	cwd = realloc(cwd, strlen(cwd) + 2);
	if (cwd[0] == '\0') {
		cwd[0] = '/';
		cwd[1] = '\0';
	}
	chdir(cwd);
}


void enter_dir(char * name) {
	char * tmp = malloc(strlen(cwd) + strlen(name) + 2);

	if (strcmp(cwd, "/"))
		sprintf(tmp, "%s/%s", cwd, name);
	else
		sprintf(tmp, "%s%s", cwd, name);

	free(cwd);
	cwd = tmp;
	chdir(cwd);
}


void remove_marked(void) {
	for (size_t i = 0; i < n_dir_entries; i++) {
		if (dir_entries[i]->marked) {
			char p[strlen(cwd) + strlen(dir_entries[i]->name) + 1];
			sprintf(p, "%s/%s", cwd, dir_entries[i]->name);
			if (remove(p) == 0) n_marked_files--;
		}
	}
}


char * mode_to_s(struct dir_entry_t * f) {
	char * s = malloc(11);
	char * p = s;
	uint16_t mode = f->mode;

	char x = 'x';
	if (f->mode & M_SUID) x = 's';
	
	if (f->file_type == FILE_DIR) *p++ = 'd';
	else *p++ = '.';

	if (MOWNER(mode) & M_READ) *p++ = 'r';
	else *p++ = '-';
	if (MOWNER(mode) & M_WRITE) *p++ = 'w';
	else *p++ = '-';
	if (MOWNER(mode) & M_EXEC) *p++ = x;
	else if (mode & M_SUID) *p++ = 'S';
	else *p++ = '-';

	if (MGROUP(mode) & M_READ) *p++ = 'r';
	else *p++ = '-';
	if (MGROUP(mode) & M_WRITE) *p++ = 'w';
	else *p++ = '-';
	if (MGROUP(mode) & M_EXEC) *p++ = x;
	else if (mode & M_SUID) *p++ = 'S';
	else *p++ = '-';

	if (mode & M_READ) *p++ = 'r';
	else *p++ = '-';
	if (mode & M_WRITE) *p++ = 'w';
	else *p++ = '-';
	if (mode & M_EXEC) { if (mode & M_SUID) *p++ = 't'; else *p++ = 'x'; }
	else if (mode & M_SUID) *p++ = 'T';
	else *p++ = '-';

	*p++ = 0;

	return s;
}

