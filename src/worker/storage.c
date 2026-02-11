#include "../../include/worker/storage.h"

#include <stdio.h>

#include "../../include/common/net.h"
#include "../../include/common/utils.h"

static struct node map[MAPSIZE];
static struct node bmap[MAPSIZE];

void init_map() {
  memset(map, 0, sizeof(map));
  memset(bmap, 0, sizeof(bmap));
}

char* get(const char* key, i8 backup) {
  i64 idx = hash(key) % MAPSIZE;
  char* k;
  struct node* curr;
  if (backup == 1) {
    curr = &bmap[idx];
  } else {
    curr = &map[idx];
  }
  if (curr->key == NULL) return NULL;

  while (curr != NULL && curr->key != NULL) {
    if (strcmp(key, curr->key) == 0) {
      return curr->val;
    }
    curr = curr->next;
  }

  return NULL;
}

STAT post(const char* key, const i32 keylen, const char* val, const i32 vallen,
          i8 backup) {
  i64 idx = hash(key) % MAPSIZE;
  struct node* curr;
  if (backup == 1) {
    curr = &bmap[idx];
  } else {
    curr = &map[idx];
  }
  if (curr->key == NULL) {
    curr->key = safestrgen(key, keylen);
    curr->val = safestrgen(val, vallen);
    curr->next = NULL;
    return OK;
  }

  struct node* prev = curr;

  while (curr != NULL) {
    if (strcmp(curr->key, key) == 0) {
      free(curr->val);
      curr->val = safestrgen(val, vallen);
      return OK;
    }
    prev = curr;
    curr = curr->next;
  }

  struct node* new = malloc(sizeof(struct node));
  if (!new) return FAIL;

  new->key = safestrgen(key, keylen);
  new->val = safestrgen(val, vallen);
  new->next = NULL;

  if (prev != NULL) {
    prev->next = new;
  }

  return OK;
}

void del(const char* key, i8 backup) {
  i64 idx = hash(key) % MAPSIZE;

  struct node* head;

  if (backup == 1) {
    head = &bmap[idx];
  } else {
    head = &map[idx];
  }
  if (head->key == NULL) return;

  if (strcmp(head->key, key) == 0) {
    if (head->val) free(head->val);
    if (head->key) free(head->key);

    if (head->next != NULL) {
      struct node* next = head->next;

      head->next = next->next;
      head->val = next->val;
      head->key = next->key;

      free(next);
    } else {
      memset(head, 0, sizeof(struct node));
    }
    return;
  }

  struct node* curr = head->next;
  struct node* prev = head;

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

void save(const char* file, int flag) {
  FILE* fp = fopen(file, "w");
  int n;

  if (!fp) {
    perror("file cannot open/create");
    return;
  }

  for (n = 0; n < MAPSIZE; ++n) {
    struct node* nod;
    if (flag == 1) {
      nod = &bmap[n];
    } else {
      nod = &map[n];
    }

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

void load(const char* file, int flag) {
  FILE* fp = fopen(file, "r");
  if (!fp) {
    perror("file cannot open/found");
    return;
  }

  i32 klen, vlen;

  printf("load ediliyor file:%s\n", file);
  while (fread(&klen, sizeof(i32), 1, fp) == 1) {
    fread(&vlen, sizeof(i32), 1, fp);
    char* pkey = (char*)malloc(klen + 1);
    char* pval = (char*)malloc(vlen + 1);
    if (!pkey || !pval) {
      printf("malloc\n");
      break;
    }
    fread(pkey, klen, 1, fp);
    fread(pval, vlen, 1, fp);

    pkey[klen] = '\0';
    pval[vlen] = '\0';

    post(pkey, klen, pval, vlen, flag);

    printf("load edildi %s %s\n", pkey, pval);
    free(pkey);
    free(pval);
  }

  fclose(fp);
}

void dump_backup(sfd fd) {
  int i;
  for (i = 0; i < MAPSIZE; ++i) {
    struct node* curr = &bmap[i];

    while (curr != NULL && curr->key != NULL) {
      struct prihdr pkt;
      pkt.pub.op = OP_POST;
      pkt.pub.keylen = strlen(curr->key);
      pkt.pub.vallen = strlen(curr->val);
      pkt.flag = 0;  // Geri yüklenen veri artık Primary olacak

      i8 hbuf[PRIHDR_SIZE];
      serialize_prihdr(&pkt, hbuf);
      send_hdr(fd, hbuf, PRIHDR_SIZE);

      // Payload (Key + Val) Gönder
      send_all(fd, curr->key, pkt.pub.keylen);
      send_all(fd, curr->val, pkt.pub.vallen);
      curr = curr->next;
    }
  }
}
