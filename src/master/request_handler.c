#include "../../include/master/request_handler.h"

void handle_client_message(sfd client_fd) {
	struct connection *conn = get_connection(client_fd);
	if (!conn) return;

	if (conn->state == CONN_WAIT_HEADER) {
		i32 toread = 12 - conn->header_bytes_read;
		i32 n = recv(client_fd, conn->header_buf + conn->header_bytes_read, toread, 0);

		if (n <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return;
			}
			perror("recv header");
			close(client_fd);
			free_connection(client_fd);
			return;
		}
		conn->header_bytes_read += n;

		if (conn->header_bytes_read == 12) {
			deserialize_hdr(&conn->header, conn->header_buf);

			conn->payload_size = conn->header.keylen + conn->header.vallen;
			conn->payload_buf = (i8 *)malloc(conn->payload_size);
			conn->payload_bytes_read = 0;

			conn->state = CONN_WAIT_PAYLOAD;
			return;
		}
	}
	if (conn->state == CONN_WAIT_PAYLOAD) {
		i32 toread = conn->payload_size - conn->payload_bytes_read;
		i32 n = recv(client_fd, conn->payload_buf + conn->payload_bytes_read, toread, 0);

		if (n <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return;
			}
			perror("recv payload");
			close(client_fd);
			free_connection(client_fd);
			return;
		}

		conn->payload_bytes_read += n;

		if (conn->payload_bytes_read == conn->payload_size) {
			struct worker *w = get_target_worker();

			conn->workerfd = w->sockfd;
			struct pay p;

			p.key = conn->payload_buf;
			p.val = conn->payload_buf + conn->header.keylen;

			printf("data completely taken. key: %s -> related worker: %d\n", p.key, w->sockfd);

			send_hdr(w->sockfd, &conn->header);
			send_pay(w->sockfd, &p, conn->header.keylen, conn->header.vallen);

			close(client_fd);
			free_connection(client_fd);
			// clear_connection(client_fd);
			return;
		}
	}
}

/**
 * SENARYO C: WORKER CEVAP DÖNDÜ
 * * Epoll, bir Worker fd'sinde EPOLLIN verirse bu çağrılır.
 * 1. Worker'dan gelen cevabı (OK/FAIL + Value) okur.
 * 2. Bu cevabın asıl sahibi olan Client'a iletir.
 */
void handle_worker_message(sfd worker_fd) {}