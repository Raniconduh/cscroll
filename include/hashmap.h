#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdint.h>

#ifndef HASHMAP_DEFAULT_CAPACITY
#define HASHMAP_DEFAULT_CAPACITY 4
#endif
// load factor percentage at which to grow
#ifndef HASHMAP_GROW_FACTOR
#define HASHMAP_GROW_FACTOR 75
#endif
// load factor percentage at which to shrink
#ifndef HASHMAP_SHRINK_FACTOR
#define HASHMAP_SHRINK_FACTOR 15
#endif

typedef struct hashmap hashmap;

typedef struct {
	char * key;
	void * val;

	/* INTERNAL */
	size_t idx;
	struct hashmap_list * node;
} hashmap_walk_t;

hashmap * hashmap_new(void (*val_destroyer)(void * elem));
void hashmap_set(hashmap * h, const char * key, void * val);
void * hashmap_get(const hashmap * h, const char * key);
void hashmap_del(hashmap * h, const char * key);
void hashmap_clear(hashmap * h);
void hashmap_free(hashmap * h);
int hashmap_walk(const hashmap * h, hashmap_walk_t * state);
size_t hashmap_size(const hashmap * h);


#endif /* HASHMAP_H */
