#include "../../include/common/connection.h"

static struct connection conns[MAX_CLIENTS];

void init_connections() {
	memset(conns, 0, sizeof(conns));
	for (int i = 0; i < MAX_CLIENTS; i++) {
		conns[i].fd = -1;
	}
}

struct connection* create_connection(sfd fd) {
	struct connection* c = &conns[fd];

	memset(c, 0, sizeof(struct connection));
	c->fd = fd;
	c->state = CONN_WAIT_HEADER;
	return c;
}

struct connection* get_connection(sfd fd) {
	if (fd < 0 || fd >= MAX_CLIENTS) return (struct connection*)0;

	if (conns[fd].fd == -1) return (struct connection*)0;
	return &conns[fd];
}

void clear_connection(sfd fd) {
	struct connection* c = &conns[fd];

	c->state = CONN_WAIT_HEADER;
	c->header_bytes_read = 0;
	c->payload_bytes_read = 0;
	c->payload_size = 0;

	memset(c->header_buf, 0, 12);

	free(c->payload_buf);

	c->payload_buf = NULL;
}

void free_connection(sfd fd) {
	struct connection* c = &conns[fd];
	if (c->payload_buf) {
		free(c->payload_buf);
		c->payload_buf = NULL;
	}
	c->fd = -1;
	c->state = CONN_IDLE;
}
