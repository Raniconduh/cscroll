#ifndef DIR_H
#define DIR_H

#include <stdbool.h>

int list_dir(char *);
void free_dir_entries(void);
void cd_back(void);
void enter_dir(char *);
void remove_marked(void);


enum file_type_t {
	FILE_REG,
	FILE_DIR,
	FILE_FIFO,
	FILE_LINK,
	FILE_BLK,
	FILE_SOCK,
	FILE_UNKNOWN
};

struct dir_entry_t {
	char * name;
	enum file_type_t file_type;
	enum file_type_t under_link;
	bool exec;
	bool marked;
};


// number of directory entries
extern size_t n_dir_entries;
// actual directory entries
extern struct dir_entry_t ** dir_entries;

// current working directory
extern char * cwd;

extern bool show_dot_files;
extern bool permission_denied;

#endif /* DIR_H */
