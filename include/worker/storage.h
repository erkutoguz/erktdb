#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
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

struct fentry {
	i32 keylen;
	i32 vallen;
	char *key;
	char *val;
};

void init_map();

char *get(const char *key);
STAT post(const char *key, const i32 keylen, const char *val, const i32 vallen);
void del(const char *key);

void save(const char *file);
void load(const char *file);

#endif