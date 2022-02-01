#ifndef _OPTS_H
#define _OPTS_H

#include <stdbool.h>
#include <stdint.h>

// RGB values can be up to 10 bits (<= 1000)
#define RGB(R, G, B) (((R << 20) | (G << 10)) | B)
#define GET_RGB(C) ((C >> 20) & 0x3FF), ((C >> 10) & 0x3FF), (C & 0x3FF)

#define ICONS_VAR "icons"
extern bool show_icons;
#define DOTS_VAR "dots"
extern bool show_dot_files;
#define COLOR_VAR "color"
extern bool color;
#define LONG_VAR "long"
extern bool p_long;
#define DIR_COLOR_VAR "dir_color"
extern uint32_t dir_color;
#define REG_COLOR_VAR "reg_color"
extern uint32_t reg_color;
#define FIFO_COLOR_VAR "fifo_color"
extern uint32_t fifo_color;
#define LINK_COLOR_VAR "link_color"
extern uint32_t link_color;
#define BLK_COLOR_VAR "block_color"
extern uint32_t blk_color;
#define SOCK_COLOR_VAR "sock_color"
extern uint32_t sock_color;
#define UNKNOWN_COLOR_VAR "unknown_color"
extern uint32_t unknown_color;
#define EXEC_COLOR_VAR "exec_color"
extern uint32_t exec_color;
#define MEDIA_COLOR_VAR "media_color"
extern uint32_t media_color;
#define ARCHIVE_COLOR_VAR "archive_color"
extern uint32_t archive_color;

bool check_config(void);
void create_config(void);
void read_config(void);
void terminate_opts(void);

#endif /* _OPTS_H */
