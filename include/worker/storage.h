#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/common/protocol.h"
#include "../../include/common/types.h"

#define MAPSIZE 4096
#define DBFILE "erkt.db"
#define BCDBFILE "erkt.db.bc"

struct node {
  char* key;
  char* val;
  struct node* next;
};

struct fentry {
  i32 keylen;
  i32 vallen;
  char* key;
  char* val;
};

void init_map();

char* get(const char* key, i8 backup);
STAT post(const char* key, const i32 keylen, const char* val, const i32 vallen,
          i8 backup);
void del(const char* key, i8 backup);

void save(const char* file, int flag);
void load(const char* file, int flag);

void dump_backup(sfd fd);

#endif
