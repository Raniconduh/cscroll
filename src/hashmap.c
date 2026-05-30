#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "hashmap.h"

/* strdup() is a POSIX feature; if not available, provide alternative */
#if (defined(_XOPEN_SOURCE)   && (_XOPEN_SOURCE >= 500)) || \
    (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L))
/* do nothing */
#else
static char * my_strdup(const char * s) {
	return strcpy(malloc(strlen(s) + 1), s);
}
#undef strdup
#define strdup my_strdup
#endif

struct hashmap {
	size_t capacity;
	size_t size;
	struct hashmap_list ** buckets;

	void (*val_destroyer)(void*);
};

struct hashmap_list {
	struct hashmap_list * next;

	uint32_t hash;

	char * key;
	void * val;
};

typedef struct hashmap_list node_t;

static uint32_t hashs(const char * s);
static node_t ** hashmap_new_buckets(size_t capacity);
static void hashmap_resize(hashmap * h, size_t newcapacity);

hashmap * hashmap_new(void (*val_destroyer)(void*)) {
	hashmap * h = malloc(sizeof(hashmap));
	h->capacity = HASHMAP_DEFAULT_CAPACITY;
	h->size = 0;
	h->buckets = hashmap_new_buckets(HASHMAP_DEFAULT_CAPACITY);
	h->val_destroyer = val_destroyer;
	return h;
}

void hashmap_set(hashmap * h, const char * key, void * val) {
	if ((h->size + 1) * 100 / h->capacity >= HASHMAP_GROW_FACTOR) {
		hashmap_resize(h, 2 * h->capacity);
	}

	uint32_t hash = hashs(key);
	size_t idx = hash % h->capacity;

	node_t * node = NULL;
	for (node_t * n = h->buckets[idx]; n; n = n->next) {
		if (n->hash == hash && !strcmp(n->key, key)) {
			node = n;
			break;
		}
	}

	if (!node) {
		node = malloc(sizeof(node_t));
		node->next = NULL;
		node->hash = hash;
		node->key = strdup(key);
		node->val = val;

		if (h->buckets[idx]) node->next = h->buckets[idx];
		h->buckets[idx] = node;
		h->size++;
	} else {
		if (h->val_destroyer) h->val_destroyer(node->val);
		node->val = val;
	}
}

void * hashmap_get(const hashmap * h, const char * key) {
	uint32_t hash = hashs(key);
	size_t idx = hash % h->capacity;

	for (node_t * n = h->buckets[idx]; n; n = n->next) {
		if (n->hash == hash && !strcmp(n->key, key)) return n->val;
	}

	return NULL;
}

void hashmap_del(hashmap * h, const char * key) {
	uint32_t hash = hashs(key);
	size_t idx = hash % h->capacity;

	node_t * node = NULL;
	node_t * prev = NULL;
	for (node_t * n = h->buckets[idx]; n; n = n->next) {
		if (n->hash == hash && !strcmp(n->key, key)) {
			node = n;
			break;
		}
		prev = n;
	}

	if (!node) return;
	if (prev) prev->next = node->next;
	else h->buckets[idx] = node->next;

	if (h->val_destroyer) h->val_destroyer(node->val);
	free(node->key);
	free(node);

	h->size--;
	if (h->size != 0
			&& h->capacity > HASHMAP_DEFAULT_CAPACITY
			&& h->size * 100 / h->capacity <= HASHMAP_SHRINK_FACTOR) {
		hashmap_resize(h, h->capacity / 2);
	}
}

void hashmap_clear(hashmap * h) {
	for (size_t i = 0; i < h->capacity; i++) {
		for (node_t * node = h->buckets[i]; node;) {
			node_t * next = node->next;

			if (h->val_destroyer) h->val_destroyer(node->val);
			free(node->key);
			free(node);

			node = next;
		}
	}

	free(h->buckets);
	h->capacity = HASHMAP_DEFAULT_CAPACITY;
	h->size     = 0;
	h->buckets  = hashmap_new_buckets(HASHMAP_DEFAULT_CAPACITY);
}

void hashmap_free(hashmap * h) {
	for (size_t i = 0; i < h->capacity; i++) {
		for (node_t * node = h->buckets[i]; node;) {
			node_t * next = node->next;

			if (h->val_destroyer) h->val_destroyer(node->val);
			free(node->key);
			free(node);

			node = next;
		}
	}

	free(h->buckets);
	free(h);
}

int hashmap_walk(const hashmap * h, hashmap_walk_t * state) {
	for (size_t i = state->idx; i < h->capacity; i++) {
		if (!h->buckets[i]) continue;
		if (state->node && !state->node->next) {
			state->node = NULL;
			continue;
		}

		node_t * node;
		if (!state->node) node = h->buckets[i];
		else node = state->node->next;
		state->node = node;
		state->key = node->key;
		state->val = node->val;

		state->idx = i;

		return 1;
	}

	return 0;
}

size_t hashmap_size(const hashmap * h) {
	return h->size;
}

/* INTERNAL */

/* FNV-1a string hash */
static uint32_t hashs(const char * s) {
	uint32_t hash = 2166136261UL;
	int c;
	while ((c = *s++)) {
		hash ^= c;
		hash *= 16777619UL;
	}
	return hash;
}

static node_t ** hashmap_new_buckets(size_t capacity) {
	node_t ** buckets;
	buckets = malloc(sizeof(node_t*) * capacity);
	for (size_t i = 0; i < capacity; i++) buckets[i] = NULL;
	return buckets;
}

static void hashmap_resize(hashmap * h, size_t newcapacity) {
	node_t ** newbuckets = hashmap_new_buckets(newcapacity);

	for (size_t i = 0; i < h->capacity; i++) {
		for (node_t * node = h->buckets[i]; node;) {
			node_t * next = node->next;

			size_t newidx = node->hash % newcapacity;
			node->next = NULL;
			if (newbuckets[newidx]) node->next = newbuckets[newidx];
			newbuckets[newidx] = node;

			node = next;
		}
	}

	free(h->buckets);
	h->buckets = newbuckets;
	h->capacity = newcapacity;
}
