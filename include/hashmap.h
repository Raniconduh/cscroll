#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stddef.h>


// collision list
struct hashmap_col_list {
	struct hashmap_col_list * next;
	struct hashmap_col_list * prev;

	unsigned long hash;
	char * key;
	void * val;
};


struct hashmap_bucket {
	struct hashmap_col_list * entries;
	struct hashmap_col_list * last;
};


typedef struct {
	void (*destroyer)(void*);
	struct hashmap_bucket ** buckets;
	size_t len; // total entries
	size_t max;
} hashmap;


typedef struct {
	size_t curbuck;
	struct hashmap_col_list * col;

	char * key;
	void * val;
} hashmap_walk_state;


hashmap * hashmap_new(void (*destroyer)(void*));
void hashmap_insert(hashmap *, char *, void *);
void * hashmap_get(const hashmap *, const char *);
void hashmap_remove(hashmap *, char *);
int hashmap_walk(hashmap *, hashmap_walk_state *);
void hashmap_destroy(hashmap *);

#endif /* HASHMAP_H */
