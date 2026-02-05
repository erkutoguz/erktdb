#include "../../include/worker/storage.h"

#include <string.h>

#include "../../include/common/utils.h"

static struct node map[MAPSIZE];

void init_map() { memset(map, 0, sizeof(map)); }

char *safestrgen(char *str, i32 len) {
	char *g = malloc(len + 1);
	if (!g) return NULL;
	memcpy(g, str, len);
	g[len] = '\0';
	return g;
}

char *get(const char *key) {
	i64 idx = hash(key) % MAPSIZE;
	char *k;

	struct node *curr = &map[idx];
	if (curr->key == NULL) return NULL;

	while (curr != NULL && curr->key != NULL) {
		if (strcmp(key, curr->key) == 0) {
			return curr->val;
		}
		curr = curr->next;
	}

	return NULL;
}

STAT post(const char *key, const i32 keylen, const char *val, const i32 vallen) {
	i64 idx = hash(key) % MAPSIZE;

	struct node *n = &map[idx];

	if (n->key == NULL) {
		n->key = safestrgen(key, keylen);
		n->val = safestrgen(val, vallen);
		n->next = NULL;
		return OK;
	}

	while (n->next != NULL) {
		if (strcmp(n->key, key) == 0) {
			free(n->val);
			n->val = safestrgen(val, vallen);
			return OK;
		}
		n = n->next;
	}

	struct node *new = malloc(sizeof(struct node));
	if (!new) return FAIL;

	new->key = safestrgen(key, keylen);
	new->val = safestrgen(val, vallen);
	new->next = NULL;

	n->next = new;

	return OK;
}