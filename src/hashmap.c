#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hashmap.h"

#define HASHMAP_RESIZE_LEN 128
#define HASHMAP_RESIZE_FACT 75 // percent full

// FNV-1a
static unsigned long hashs(const char * s) {
	unsigned long hash = 2166136261UL;
	int c;
	while ((c = *s++)) {
		hash *= 2166136261UL;
		hash ^= c;
	}
	return hash;
}


static struct hashmap_bucket * new_bucket(void) {
	struct hashmap_bucket * b = malloc(sizeof(struct hashmap_bucket));
	b->entries = NULL;
	b->last = NULL;
	return b;
}


static struct hashmap_col_list * new_col_list(void) {
	struct hashmap_col_list * cl = malloc(sizeof(struct hashmap_col_list));
	cl->next = NULL;
	cl->prev = NULL;
	cl->hash = 0;
	cl->key = NULL;
	cl->val = NULL;
	return cl;
}


static void destroy_col(struct hashmap_col_list * cl) {
	free(cl->key);
	free(cl);
}


static struct hashmap_col_list * append_col(struct hashmap_bucket * b) {
	if (!b->entries) {
		b->entries = new_col_list();
		b->last = b->entries;
		return b->entries;
	}

	b->last->next = new_col_list();
	b->last->next->prev = b->last;
	b->last = b->last->next;
	return b->last;
}


static struct hashmap_bucket * hashmap_get_bucket(const hashmap * hm, const char * key) {
	unsigned long hash = hashs(key);
	size_t index = hash % hm->max;
	return hm->buckets[index];
}


static struct hashmap_col_list * search_col_list(struct hashmap_bucket * b, const char * key) {
	for (struct hashmap_col_list * cl = b->entries; cl; cl = cl->next) {
		if (!strcmp(key, cl->key)) return cl;
	}
	return NULL;
}


hashmap * hashmap_new(void (*destroyer)(void*)) {
	hashmap * hm = malloc(sizeof(hashmap));
	hm->destroyer = destroyer;
	hm->buckets = malloc(sizeof(struct hashmap_bucket*) * HASHMAP_RESIZE_LEN);
	hm->len = 0;
	hm->max = HASHMAP_RESIZE_LEN;

	for (size_t i = 0; i < hm->max; i++) {
		hm->buckets[i] = NULL;
	}

	return hm;
}


static void hashmap_resize(hashmap * hm) {
	size_t nmax = hm->max * 2;
	struct hashmap_bucket ** nb = malloc(sizeof(struct hashmap_bucket*) * nmax);

	for (size_t i = 0; i < nmax; i++) {
		nb[i] = NULL;
	}

	for (size_t i = 0; i < hm->max; i++) {
		struct hashmap_bucket * ob = hm->buckets[i];
		if (!ob || !ob->entries) continue;

		for (struct hashmap_col_list * cl = ob->entries; cl;) {
			struct hashmap_col_list * next = cl->next;

			size_t nindex = cl->hash % nmax;
			if (!nb[nindex]) {
				nb[nindex] = new_bucket();
				nb[nindex]->entries = cl;
				nb[nindex]->last = cl;
				cl->prev = NULL;
				cl->next = NULL;
			} else {
				struct hashmap_col_list * last = nb[nindex]->last;
				last->next = cl;
				cl->next = NULL;
				cl->prev = last;
				nb[nindex]->last = cl;
			}

			cl = next;
		}

		free(ob);
	}

	free(hm->buckets);
	hm->buckets = nb;
	hm->max = nmax;
}


void hashmap_insert(hashmap * hm, char * key, void * val) {
	if ((hm->len * 100) / hm->max >= HASHMAP_RESIZE_FACT) {
		hashmap_resize(hm);
	}

	struct hashmap_col_list * cl = NULL;

	unsigned long hash = hashs(key);
	size_t index = hash % hm->max;
	if (!hm->buckets[index]) {
		hm->buckets[index] = new_bucket();
	} else {
		for (struct hashmap_col_list * l = hm->buckets[index]->entries; l; l = l->next) {
			// overrite the old collision entry
			if (!strcmp(l->key, key)) {
				cl = l;
				if (hm->destroyer) hm->destroyer(cl->val);
				cl->val = val;
			}
		}
	}

	if (!cl) {
		cl = append_col(hm->buckets[index]);
		cl->hash = hash;
		cl->key = strdup(key);
		cl->val = val;

		hm->len++;
	}
}


void * hashmap_get(const hashmap * hm, const char * key) {
	struct hashmap_bucket * b = hashmap_get_bucket(hm, key);
	if (!b) return NULL;
	struct hashmap_col_list * cl = search_col_list(b, key);
	if (!cl) return NULL;
	return cl->val;
}


void hashmap_remove(hashmap * hm, char * key) {
	struct hashmap_bucket * b = hashmap_get_bucket(hm, key);
	if (!b) return;

	struct hashmap_col_list * cl = search_col_list(b, key);
	if (!cl) return;

	struct hashmap_col_list * prev = cl->prev;
	struct hashmap_col_list * next = cl->next;

	if (!prev) b->entries = next;
	else prev->next = next;
	if (next) next->prev = prev;

	if (hm->destroyer) hm->destroyer(cl->val);
	destroy_col(cl);
}


int hashmap_walk(hashmap * hm, hashmap_walk_state * state) {
	for (size_t i = state->curbuck; i < hm->max; i++) {
		struct hashmap_bucket * b = hm->buckets[i];
		if (!b || !b->entries) continue;

		if (!state->col) state->col = b->entries;
		for (struct hashmap_col_list * cl = state->col; cl; cl = cl->next) {
			state->col = cl->next;
			state->curbuck = i;
			if (!state->col) state->curbuck++;

			state->key = cl->key;
			state->val = cl->val;
			return 1;
		}
		state->col = NULL;
	}

	return 0;
}


void hashmap_destroy(hashmap * hm) {
	for (size_t i = 0; i < hm->max; i++) {
		struct hashmap_bucket * b = hm->buckets[i];
		if (!b) continue;

		// free each collision in the bucket
		for (struct hashmap_col_list * cl = b->entries; cl;) {
			struct hashmap_col_list * next = cl->next;
			if (hm->destroyer) hm->destroyer(cl->val);
			destroy_col(cl);
			cl = next;
		}

		free(b);
	}

	free(hm->buckets);
	free(hm);
}
