#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#include "dir.h"

char * cwd = NULL;

size_t n_dir_entries = 0;
struct dir_entry_t ** dir_entries = NULL;

int list_dir(char * dir_path) {
	struct dirent * d_entry;
	DIR * dir = opendir(dir_path);

	n_dir_entries = 0;

	if (!dir)
		return 1;

	while ((d_entry = readdir(dir))) {
		struct dir_entry_t * dir_entry = malloc(sizeof(struct dir_entry_t) + 12);

		char * d_name = d_entry->d_name;
		size_t d_name_len = strlen(d_name);

		if (!strcmp(d_name, ".") || !strcmp(d_name, "..")) continue;

		struct stat * buf = malloc(sizeof(struct stat));
		char * tmp_path = malloc(d_name_len + strlen(dir_path) + 2);
		sprintf(tmp_path, "%s/%s", dir_path, d_name);
		stat(tmp_path, buf);
		free(tmp_path);

		dir_entries = realloc(dir_entries, sizeof(struct dir_entry_t) * (n_dir_entries + 1));
		dir_entry->name = malloc(d_name_len + 1);
		
		strcpy(dir_entry->name, d_name);

		mode_t s = buf->st_mode;
		if (S_ISBLK(s) || S_ISCHR(s)) dir_entry->file_type = FILE_BLK;
		else if (S_ISDIR(s)) dir_entry->file_type = FILE_DIR;
		else if (S_ISFIFO(s)) dir_entry->file_type = FILE_FIFO;
		else if (S_ISLNK(s)) dir_entry->file_type = FILE_LINK;
		else dir_entry->file_type = FILE_REG;
		free(buf);

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
}


void enter_dir(char * name) {
	cwd = realloc(cwd, strlen(cwd) + strlen(name) + 2);
	if (strcmp(cwd, "/"))
		sprintf(cwd, "%s/%s", cwd, name);
	else
		strcat(cwd, name);
}
