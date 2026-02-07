#include "../../include/master/request_handler.h"

#include "../../include/common/utils.h"

i64 uuid = 0;

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
			deserialize_pubhdr(&conn->header.pub, conn->header_buf);

			conn->payload_size = conn->header.pub.keylen + conn->header.pub.vallen;
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
			const char *k = safestrgen(conn->payload_buf, conn->header.pub.keylen);
			struct worker *w = get_target_worker(k);

			conn->workerfd = w->sockfd;
			struct pay p;

			p.key = conn->payload_buf;
			p.val = conn->payload_buf + conn->header.pub.keylen;

			printf("data completely taken. key: %s -> related worker: %d\n", p.key, w->sockfd);

			i8 buf[24];

			struct prihdr pri;
			pri.pub = conn->header.pub;
			pri.uuid = uuid++;
			pri.client_fd = client_fd;
			serialize_prihdr(&pri, buf);

			send_hdr(w->sockfd, buf, 24);
			send_pay(w->sockfd, &p, conn->header.pub.keylen, conn->header.pub.vallen);

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