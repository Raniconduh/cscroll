#ifndef _OPTS_H
#define _OPTS_H

#include <stdbool.h>


#define ICONS_VAR "icons"
extern bool show_icons;
extern bool show_dot_files;
#define COLOR_VAR "color"
extern bool color;
#define LONG_VAR "long"
extern bool p_long;


bool check_config(void);
void create_config(void);
void read_config(void);
void terminate_opts(void);

#endif /* _OPTS_H */
