#include "../../include/worker/storage.h"

#include "../../include/common/utils.h"

static struct node map[MAPSIZE];

void init_map() { memset(map, 0, sizeof(map)); }

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

	struct node *curr = &map[idx];

	if (curr->key == NULL) {
		curr->key = safestrgen(key, keylen);
		curr->val = safestrgen(val, vallen);
		curr->next = NULL;
		return OK;
	}

	struct node *prev = curr;

	while (curr != NULL) {
		if (strcmp(curr->key, key) == 0) {
			free(curr->val);
			curr->val = safestrgen(val, vallen);
			return OK;
		}
		prev = curr;
		curr = curr->next;
	}

	struct node *new = malloc(sizeof(struct node));
	if (!new) return FAIL;

	new->key = safestrgen(key, keylen);
	new->val = safestrgen(val, vallen);
	new->next = NULL;

	if (prev != NULL) {
		prev->next = new;
	}

	return OK;
}