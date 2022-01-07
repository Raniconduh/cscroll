#ifndef TYPE_H
#define TYPE_H

#define ICON_DIR "\uf74a"
#define ICON_GEAR "\uf013"
#define ICON_GENERIC "\uf15b"

#include "dir.h"

struct icon_pair {
	char * ext;
	char * icon;
};


char * get_ext(char *);
void lowers(char *);
enum mime_type_t get_mime(char *);
char * get_icon(char *);

#endif /* TYPE_H */
