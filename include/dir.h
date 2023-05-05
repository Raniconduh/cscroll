#ifndef _DIR_H
#define _DIR_H

#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define MOWNER(M) (M >> 8)
#define MGROUP(M) (M >> 4)
#define POWNER(M) (M << 8)
#define PGROUP(M) (M << 4)

#define M_EXEC  (1 << 0)
#define M_WRITE (1 << 1)
#define M_READ  (1 << 2)
#define M_SUID  (1 << 3)

enum file_type_t {
	FILE_REG,
	FILE_DIR,
	FILE_FIFO,
	FILE_LINK,
	FILE_BLK,
	FILE_CHR,
	FILE_SOCK,
	FILE_UNKNOWN
};

enum mime_type_t {
	MIME_UNKNOWN,
	MIME_MEDIA,
	MIME_ARCHIVE
};

enum f_size {
	B  = 0,
	KB = 1,
	MB = 2,
	GB = 3,
	TB = 4,
	PB = 5,
};

struct dir_entry_t {
	char * name;
	enum file_type_t file_type;
	enum file_type_t under_link;
	enum mime_type_t m_type;
	bool marked;

	uint16_t mode;
	time_t mtime;
	long owner;
	long group;
	size_t size;
	enum f_size u_size;

	// oneshot args
	bool last_in_col;
};


int list_dir(char *);
void free_dir_entries(void);
void cd_back(void);
void enter_dir(char *);
int remove_file(struct dir_entry_t *);
void remove_marked(void);
char * mode_to_s(struct dir_entry_t *);
bool check_dpath(char *);
struct dir_entry_t * gen_dir_entry(char *, char *);
void get_home(void);


// number of directory entries
extern size_t n_dir_entries;
// longest owner name in dir
extern size_t dir_longest_owner;
// longest group name in dir
extern size_t dir_longest_group;
// actual directory entries
extern struct dir_entry_t ** dir_entries;

// current working directory
extern char * cwd;
extern char * homedir;
extern size_t homedir_len;
extern bool in_home_subdir;

extern bool permission_denied;
extern bool cwd_is_file;

#endif /* _DIR_H */
