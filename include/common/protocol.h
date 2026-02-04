#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "types.h"

enum OP { OP_GET = 0x0, OP_POST = 0x1 };
typedef enum { OK = 0, FAIL = -1 } STAT;

struct hdr {
	i16 magic;	 // 2
	enum OP op : 8;	 // 1
	i8 pad;		 // 1
	i32 keylen;	 // 4
	i32 vallen;	 // 4
};

struct pay {
	i8 *key;
	i8 *val;
};

#endif