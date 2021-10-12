#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdbool.h>

#include "dir.h"
#include "io.h"

char * cwd = NULL;

size_t n_dir_entries = 0;
struct dir_entry_t ** dir_entries = NULL;

bool show_dot_files = false;
bool permission_denied = false;

int list_dir(char * dir_path) {
	struct dirent * d_entry;
	DIR * dir = opendir(dir_path);

	n_dir_entries = 0;

	if (!dir) {
		permission_denied = true;
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
		// S_IXUSR
		if (dir_entry->file_type != FILE_DIR)
			dir_entry->exec = (bool)(buf->st_mode & S_IXUSR);
		else dir_entry->exec = false;
		
		free(tmp_path);
		free(buf);

		dir_entry->marked = false;

		dir_entries[n_dir_entries] = dir_entry;
		n_dir_entries++;
	}

	closedir(dir);

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
	setenv("PWD", cwd, true);
}


void enter_dir(char * name) {
	cwd = realloc(cwd, strlen(cwd) + strlen(name) + 2);
	if (strcmp(cwd, "/"))
		sprintf(cwd, "%s/%s", cwd, name);
	else
		strcat(cwd, name);
	setenv("PWD", cwd, true);
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
