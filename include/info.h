#ifndef _INFO_H
#define _INFO_H

enum info_t {
	INFO_INFO,
	INFO_WARN,
	INFO_ERR,
};


void info_init(void);
void display_info(enum info_t, char *, ...);
void refresh_info(void);
void page_info(void);

#endif /* _INFO_H */
