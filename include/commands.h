#ifndef COMMANDS_H
#define COMMANDS_H

void ext_open(char *);
long search_file(long, char *);
void create_cuts(char *, char **);
void free_cuts(void);
void paste_cuts(char *);
void run_cmd(char *);

extern bool cutting;
extern char * cut_start_dir;
extern char ** cuts;

#endif /* COMMANDS_H */
