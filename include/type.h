#ifndef TYPE_H
#define TYPE_H

char * get_ext(char *);
int scmp(const void *, const void *);
void lowers(char *);

#define n_media_exts 68
#define n_archive_exts 55

extern char * media_exts[n_media_exts];
extern char * archive_exts[n_archive_exts];

#endif
