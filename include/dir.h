#ifndef DIR_H
#define DIR_H

#include <stddef.h>
#include "cvector.h"

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
	enum de_type type;
	size_t size;
	char * name;
} dirent_t;

typedef struct {
	size_t len;
	cvector(dirent_t) entries;
} dir_t;

int dir_list(const char * path, dir_t * dir);
void dir_free(dir_t * dir);

#endif /* DIR_H */
