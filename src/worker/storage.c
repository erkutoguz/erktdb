#include "../../include/worker/storage.h"

#include <stdio.h>

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

void del(const char *key) {
	i64 idx = hash(key) % MAPSIZE;

	struct node *head = &map[idx];
	if (head->key == NULL) return;

	if (strcmp(head->key, key) == 0) {
		if (head->val) free(head->val);
		if (head->key) free(head->key);

		if (head->next != NULL) {
			struct node *next = head->next;

			head->next = next->next;
			head->val = next->val;
			head->key = next->key;

			free(next);
		} else {
			memset(head, 0, sizeof(struct node));
		}
		return;
	}

	struct node *curr = head->next;
	struct node *prev = head;

	while (curr != NULL) {
		if (strcmp(curr->key, key) == 0) {
			prev->next = curr->next;

			if (curr->val) free(curr->val);
			if (curr->key) free(curr->key);
			free(curr);
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

void save(const char *file) {
	FILE *fp = fopen(file, "w");
	int n;

	if (!fp) {
		perror("file cannot open/create");
		return;
	}

	for (n = 0; n < MAPSIZE; ++n) {
		struct node *nod = &map[n];

		if (nod->key == NULL) continue;

		while (nod != NULL) {
			if (nod->key != NULL) {
				i32 klen = strlen(nod->key);
				i32 vlen = strlen(nod->val);

				fwrite(&klen, sizeof(i32), 1, fp);
				fwrite(&vlen, sizeof(i32), 1, fp);

				fwrite(nod->key, klen, 1, fp);
				fwrite(nod->val, vlen, 1, fp);
			}
			nod = nod->next;
		}
	}

	fclose(fp);
}

void load(const char *file) {
	FILE *fp = fopen(file, "r");
	if (!fp) {
		perror("file cannot open/found");
		return;
	}

	i32 klen, vlen;

	while (fread(&klen, sizeof(i32), 1, fp) == 1) {
		fread(&vlen, sizeof(i32), 1, fp);
		char *pkey = (char *)malloc(klen + 1);
		char *pval = (char *)malloc(vlen + 1);
		if (!pkey || !pval) {
			printf("malloc\n");
			break;
		}
		fread(pkey, klen, 1, fp);
		fread(pval, vlen, 1, fp);

		pkey[klen] = '\0';
		pval[vlen] = '\0';

		post(pkey, klen, pval, vlen);

		free(pkey);
		free(pval);
	}

	fclose(fp);
}
