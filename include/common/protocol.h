#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "types.h"

enum OP { OP_GET = 0x0, OP_POST = 0x1 };
typedef enum { OK = 0, FAIL = -1 } STAT;

struct pubhdr {
	i16 magic;	 // 2
	enum OP op : 8;	 // 1
	i8 pad;		 // 1
	i32 keylen;	 // 4
	i32 vallen;	 // 4
};

struct prihdr {
	struct pubhdr pub;  // 12
	i32 client_fd;	    // 4
	i64 uuid;	    // 8
};

struct pay {
	i8 *key;
	i8 *val;
};

#endif