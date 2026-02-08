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

void reset_connection(sfd fd) {
	struct connection* conn = get_connection(fd);
	if (!conn) return;

	if (conn->bodybuf) {
		free(conn->bodybuf);
		conn->bodybuf = NULL;
	}
	conn->header_bytes_read = 0;
	conn->body_bytes_read = 0;
	conn->body_size = 0;

	conn->state = CONN_WAIT_HEADER;
}

void free_connection(sfd fd) {
	struct connection* conn = get_connection(fd);
	if (!conn) return;

	if (conn->fd != -1) {
		close(conn->fd);
	}

	if (conn->bodybuf) {
		free(conn->bodybuf);
	}

	memset(conn, 0, sizeof(struct connection));
	conn->fd = -1;
}
