#include <string.h>
#include <stdlib.h>

#include "dir.h"
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

	char * ext = malloc(strlen(t_ext) + 1);
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

#if ICONS
char * get_icon(struct dir_entry_t * f) {
	char * t_ext = get_ext(f->name);
	struct icon_pair * t = NULL;
	if (t_ext) {
		char * ext = malloc(strlen(t_ext) + 1);
		strcpy(ext, t_ext);
		lowers(ext);

		t = bsearch(&ext, icons, n_icons, sizeof(icons[0]), icmp);
		free(ext);
	}

	if (!t) {
		if (f->file_type == FILE_DIR) return ICON_DIR;
		if (f->mode & POWNER(M_EXEC)) return ICON_GEAR;
		return ICON_GENERIC;
	}
	return t->icon;
}
#endif
