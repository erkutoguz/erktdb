#include "../../include/worker/master_request_handler.h"

void handle_master_message(sfd fd) {
	struct connection *conn = get_connection(fd);

	if (!conn) return;
	if (conn->state == CONN_WAIT_HEADER) {
		i32 toread = 12 - conn->header_bytes_read;
		i32 n = recv(fd, conn->header_buf + conn->header_bytes_read, toread, 0);

		if (n <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return;
			}
			perror("recv header");
			close(fd);
			free_connection(fd);
			return;
		}

		conn->header_bytes_read += n;
		if (conn->header_bytes_read == 12) {
			deserialize_hdr(&conn->header, conn->header_buf);

			conn->payload_size = conn->header.keylen + conn->header.vallen;
			conn->payload_buf = (i8 *)malloc(conn->payload_size);
			conn->payload_bytes_read = 0;

			conn->state = CONN_WAIT_PAYLOAD;
		}
		return;
	}

	if (conn->state == CONN_WAIT_PAYLOAD) {
		i32 toread = conn->payload_size - conn->payload_bytes_read;
		i32 n = recv(fd, conn->payload_buf + conn->payload_bytes_read, toread, 0);

		if (n <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return;
			}
			perror("recv payload");
			close(fd);
			free_connection(fd);
			return;
		}

		conn->payload_bytes_read += n;

		if (conn->payload_bytes_read == conn->payload_size) {
			struct pay p;
			p.key = conn->payload_buf;
			p.val = conn->payload_buf + conn->header.keylen;

			STAT saved = OK;

			// send_master_stat(fd, saved);
			// if (saved == OK) {
			//     printf("success\n");
			// } else {
			//     printf("failed\n");
			// }
			clear_connection(fd);

			return;
		}
	}
}

STAT send_master_stat(sfd fd, STAT stat) {
	i8 buf[1];
	buf[0] = (stat >> 24) & 0xff;
	return send_all(fd, buf, sizeof(i8));
}