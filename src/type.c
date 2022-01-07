#include <string.h>
#include <stdlib.h>

#include "type.h"
#include "icons.h"


char * get_ext(char * s) {
	char * ns = strrchr(s, '.');
	if (!ns) return NULL;
	return ns + 1;
}


void lowers(char * s) {
	for (char * p = s; *p; p++)
		// A to Z
		if (*p >= 65 && *p <= 90)
			*p += 32; // convert upper to lower
}

enum mime_type_t get_mime(char * file) {
	char * t_ext = get_ext(file);
	if (!t_ext) return MIME_UNKNOWN;

	char * ext = malloc(strlen(t_ext));
	strcpy(ext, t_ext);
	lowers(ext);

	struct icon_pair * t =
		bsearch(&ext, icons, n_icons, sizeof(icons[0]), icmp);
	free(ext);
	if (!t) return MIME_UNKNOWN;

	if (!strcmp(t->icon, ICON_ARCHIVE))
		return MIME_ARCHIVE;
	else if (!strcmp(t->icon, ICON_AUDIO) ||
			 !strcmp(t->icon, ICON_VIDEO) ||
			 !strcmp(t->icon, ICON_IMAGE))
		return MIME_MEDIA;
	else return MIME_UNKNOWN;
}

char * get_icon(char * file) {
	char * t_ext = get_ext(file);
	if (!t_ext) return NULL;

	char * ext = malloc(strlen(t_ext));
	strcpy(ext, t_ext);
	lowers(ext);

	struct icon_pair * t =
		bsearch(&ext, icons, n_icons, sizeof(icons[0]), icmp);
	free(ext);
	if (!t) return NULL;
	return t->icon;
}
