#ifndef STORAGE_H
#define STORAGE_H

#include <stdlib.h>
#include <string.h>

#include "../../include/common/protocol.h"
#include "../../include/common/types.h"

#define MAPSIZE 4096

struct node {
	char *key;
	char *val;
	struct node *next;
};

void init_map();

char *safestrgen(char *str, i32 len);
char *get(const char *key);
STAT post(const char *key, const i32 keylen, const char *val, const i32 vallen);

#endif