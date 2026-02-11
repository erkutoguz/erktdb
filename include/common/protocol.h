#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "types.h"

#define PUBHDR_SIZE 12
#define PRIHDR_SIZE 25
#define RETHDR_SIZE 20

enum OP {
  OP_GET = 0x0,
  OP_POST = 0x1,
  OP_DEL = 0x2,
  OP_SYNC_START = 0x3,
  OP_SYNC_END = 0x4
};
typedef enum { OK = 0, FAIL = -1, SERV_ERR = 1 } STAT;

struct pubhdr {
  i16 magic;       // 2
  enum OP op : 8;  // 1
  i8 pad;          // 1
  i32 keylen;      // 4
  i32 vallen;      // 4
};

struct prihdr {
  struct pubhdr pub;  // 12
  i32 client_fd;      // 4
  i64 uuid;           // 8
  i8 flag;            // 1
} __attribute__((packed));

struct pay {
  i8* key;
  i8* val;
};

struct rethdr {
  i64 uuid;       // 8
  sfd client_fd;  // 4
  i32 msglen;     // 4
  STAT status;    // 4
} __attribute__((packed));
;

#endif
