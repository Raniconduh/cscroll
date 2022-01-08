#ifndef TYPE_H
#define TYPE_H

#include "dir.h"

struct icon_pair {
	char * ext;
	char * icon;
};


char * get_ext(char *);
void lowers(char *);
enum mime_type_t get_mime(char *);
char * get_icon(struct dir_entry_t *);

#endif /* TYPE_H */
