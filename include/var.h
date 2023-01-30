#ifndef _VAR_H
#define _VAR_H

#include <stdbool.h>

#define VAR_FALSE ((void*)0)
#define VAR_TRUE ((void*)1)

void var_init(void);
bool var_set(char *, void *);
void terminate_var(void);

#endif /* _VAR_H */
