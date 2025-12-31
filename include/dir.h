#ifndef DIR_H
#define DIR_H

#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "cvector.h"

#define M_SUID S_ISUID
#define M_SGID S_ISGID
#define M_STICKY S_ISVTX
#define M_USRR S_IRUSR
#define M_USRW S_IWUSR
#define M_USRX S_IXUSR
#define M_GRPR S_IRGRP
#define M_GRPW S_IWGRP
#define M_GRPX S_IXGRP
#define M_OTHR S_IROTH
#define M_OTHW S_IWOTH
#define M_OTHX S_IXOTH

enum de_type {
	DE_FILE,
	DE_DIR,
	DE_FIFO,
	DE_LINK,
	DE_BLOCK,
	DE_CHAR,
	DE_SOCKET,
	DE_UNKNOWN
};

typedef struct {
	char * name;
	enum de_type type;

	size_t size;
	uint16_t mode;
	time_t mtime;
	char * uname;
	char * gname;

	char * linkname;
	enum de_type linktype;
} dirent_t;

typedef struct {
	size_t len;
	cvector(dirent_t) entries;

	size_t longest_uname;
	size_t longest_gname;
	size_t longest_size;
} dir_t;

int dir_list(const char * path, dir_t * dir);
void dir_free(dir_t * dir);
const char * dir_get_cwd(void);
int dir_cd_back(const char * cwd);
int dir_cd(const char * cwd, const char * next);
int dir_search_name(const dir_t * dir, const char * name, size_t * idx);
const char * dir_basename(const char * path);
char dirent_crepr(const dirent_t * de); // character representing dir entry
char dirent_creprl(const dirent_t * de); // like above, but for the link
char dirent_longcrepr(const dirent_t * de); // like above, but for long mode
bool dirent_isexec(const dirent_t * de);
const char * dirent_prettymode(const dirent_t * de);

#endif /* DIR_H */
