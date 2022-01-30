#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"


// jenkins one at a time hash
static uint32_t map_hashs(char * s) {
	uint32_t hash;
	char * i;
    for(hash = 0, i = s; *i; i++)
    {
        hash += *i;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;	
}


static bucket_data * map_new_data(void) {
	bucket_data * a = malloc(sizeof(bucket_data));

	a->next = NULL;
	a->key = NULL;
	a->value = NULL;

	return a;
}


static bucket * map_new_bucket(void) {
	bucket * a = malloc(sizeof(bucket));

	a->hash = 0;
	a->data = NULL;

	return a;
}


map * map_new(size_t n) {
	map * m = malloc(sizeof(map));

	m->arr = malloc(sizeof(bucket *) * (n + 1));
	m->length = n;

	// initialize array
	for (size_t i = 0; i < n; i++) {
		m->arr[i] = NULL;
	}

	return m;
}


void map_insert(map * m, char * k, void (*v)(void *)) {
	long klen = strlen(k);
	uint32_t khash = map_hashs(k);

	// hash already exists, add to bucket
	if (m->length > 0 && m->arr[khash % m->length]) {
		bucket_data * d = map_new_data();

		d->key = malloc(klen);
		strcpy(d->key, k);

		d->value = v;

		// replace top node with new node
		d->next = m->arr[khash % m->length]->data;
		m->arr[khash % m->length]->data = d;
		return;
	}

	// need to create new bucket
	bucket_data * d = map_new_data();

	d->key = malloc(klen);
	strcpy(d->key, k);
	d->value = v;

	bucket * b = map_new_bucket();

	b->hash = khash;
	b->data = d;

	m->arr[khash % m->length] = b;

	return;
}


void map_nuke(map * m) {
	// loop over array
	for (size_t i = 0; i < m->length; i++) {
		if (!m->arr[i]) continue;
		// first free linked list
		for (bucket_data * p = m->arr[i]->data; p;) {
			bucket_data * tmp = p->next;
			free(p);
			p = tmp;
		}
		// then free bucket
		free(m->arr[i]);
	}
	// free array
	free(m->arr);
	// finally free map itself
	free(m);
}


void (*map_index(map * m, char * k))(void *) {
	uint32_t khash = map_hashs(k);
	bucket_data * p;
	// if hash exists ...
	if (m->arr[khash % m->length])
		// ... then search bucket for key
		for (p = m->arr[khash % m->length]->data; p; p = p->next)
			if (!strcmp(p->key, k)) return p->value;
	return NULL;
}
