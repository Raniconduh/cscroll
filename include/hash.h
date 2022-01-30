#ifndef _HASH_H
#define _HASH_H

#include <stdint.h>


typedef struct BucketData {
	struct BucketData * next;
	char * key;
	void (*value)(void *);
} bucket_data;

typedef struct Bucket {
	uint32_t hash;
	bucket_data * data;
} bucket;

typedef struct Map {
	uint32_t length;
	bucket ** arr; // list of pointers to buckets
} map;


map * map_new(size_t n);
void map_insert(map *, char *, void (*)(void *));
void map_nuke(map *);
void (*map_index(map *, char *))(void *);

#endif /* _HASH_H */
