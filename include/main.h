#ifndef _MAIN_H
#define _MAIN_H

#define LAST_F (n_dir_entries > ((unsigned)LINES - 6) ? ((unsigned)LINES - 6) : n_dir_entries)


void help(void);
void sig_handler(int);


extern size_t first_f, last_f, cursor;

#endif /*_MAIN_H */
