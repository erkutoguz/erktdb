#ifndef MASTER_CONNECTION_H
#define MASTER_CONNECTION_H

#include "../../include/common/net.h"
#include "../../include/common/protocol.h"

#define MAX_CLIENTS 1024

typedef enum { CONN_WAIT_HEADER, CONN_WAIT_PAYLOAD, CONN_IDLE } conn_state;
typedef enum { CONN_CLI, CONN_WOR } conn_type;

struct connection {
	sfd fd;
	conn_type type;
	conn_state state;

	i8 header_buf[sizeof(struct prihdr)];
	i32 header_bytes_read;
	i32 expected_hdr_size;

	union {
		struct pubhdr client_req;  // 12 cli -> master
		struct prihdr master_req;  // 24 master -> worker
		struct rethdr worker_res;  // 20 worker -> master
	} info;

	i8* bodybuf;
	i32 body_bytes_read;
	i32 body_size;
};

void init_connections();

struct connection* create_connection(sfd fd);

struct connection* get_connection(sfd fd);

void reset_connection(sfd fd);

void free_connection(sfd fd);

#endif