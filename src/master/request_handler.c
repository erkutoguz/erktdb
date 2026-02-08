#include "../../include/master/request_handler.h"

#include "../../include/common/connection.h"
#include "../../include/common/utils.h"

i64 uuid = 0;

void handle_client_message(sfd client_fd) {
	struct connection *conn = get_connection(client_fd);
	if (!conn) return;

	if (conn->state == CONN_WAIT_HEADER) {
		conn->type = CONN_CLI;
		conn->expected_hdr_size = sizeof(struct pubhdr);

		i32 toread = conn->expected_hdr_size - conn->header_bytes_read;
		i32 n = recv(client_fd, conn->header_buf + conn->header_bytes_read, toread, 0);

		if (n <= 0) {
			if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
			perror("client2master recv header");

			free_connection(client_fd);
			return;
		}
		conn->header_bytes_read += n;

		if (conn->header_bytes_read == conn->expected_hdr_size) {
			deserialize_pubhdr(&(conn->info.client_req), conn->header_buf);
			conn->body_size = conn->info.client_req.keylen + conn->info.client_req.vallen;
			conn->bodybuf = (i8 *)malloc(conn->body_size);
			conn->body_bytes_read = 0;
			conn->state = CONN_WAIT_PAYLOAD;
			return;
		}
	}

	if (conn->state == CONN_WAIT_PAYLOAD) {
		if (conn->body_size > conn->body_bytes_read) {
			i32 toread = conn->body_size - conn->body_bytes_read;
			i32 n = recv(client_fd, conn->bodybuf + conn->body_bytes_read, toread, 0);

			if (n <= 0) {
				if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
				perror("client2master recv payload");
				free_connection(client_fd);
				return;
			}
			conn->body_bytes_read += n;
		}

		if (conn->body_bytes_read == conn->body_size) {
			const char *k = safestrgen(conn->bodybuf, conn->info.client_req.keylen);
			struct worker *w = get_target_worker(k);

			if (!w || w->sockfd == -1) {
				fprintf(stderr, "noworker.\n");
				free_connection(client_fd);
				free(k);
				return;
			}
			free(k);

			struct pay p;
			p.key = conn->bodybuf;
			p.val = conn->bodybuf + conn->info.client_req.keylen;

			printf("data completely taken. key: %s -> related worker: %d\n", (char *)p.key, w->sockfd);

			i8 buf[24];
			struct prihdr pri;
			pri.pub = conn->info.client_req;
			pri.uuid = uuid++;
			pri.client_fd = client_fd;

			serialize_prihdr(&pri, buf);

			send_hdr(w->sockfd, buf, 24);
			send_pay(w->sockfd, &p, conn->info.client_req.keylen, conn->info.client_req.vallen);

			if (conn->bodybuf) {
				free(conn->bodybuf);
				conn->bodybuf = NULL;
			}
			return;
		}
	}
}

void handle_worker_message(sfd worker_fd) {
	struct connection *conn = get_connection(worker_fd);
	if (!conn) return;

	if (conn->state == CONN_WAIT_HEADER) {
		conn->type = CONN_WOR;
		conn->expected_hdr_size = sizeof(struct rethdr);

		i32 toread = conn->expected_hdr_size - conn->header_bytes_read;
		i32 n = recv(worker_fd, conn->header_buf + conn->header_bytes_read, toread, 0);

		if (n <= 0) {
			if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
			perror("worker2master recv header");

			// worker öldü -> yeniden bğala ya da sil
			free_connection(worker_fd);
			return;
		}
		conn->header_bytes_read += n;

		if (conn->header_bytes_read == conn->expected_hdr_size) {
			deserialize_rethdr(&(conn->info.worker_res), conn->header_buf);

			conn->body_size = conn->info.worker_res.msglen;
			if (conn->body_size > 0) {
				conn->bodybuf = (i8 *)malloc(conn->body_size + 1);
				conn->bodybuf[conn->body_size] = 0;
				conn->body_bytes_read = 0;
				conn->state = CONN_WAIT_PAYLOAD;
			} else {
				goto forwardclient;
			}
			return;
		}
	}

	if (conn->state == CONN_WAIT_PAYLOAD) {
		i32 toread = conn->body_size - conn->body_bytes_read;
		i32 n = recv(worker_fd, conn->bodybuf + conn->body_bytes_read, toread, 0);

		if (n <= 0) {
			if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
			perror("worker2master recv payload");
			free_connection(worker_fd);
			return;
		}

		conn->body_bytes_read += n;

		if (conn->body_bytes_read == conn->body_size) {
			goto forwardclient;
		}
		return;
	}

forwardclient:
	printf("Master: Worker cevabı (Status: %d). Client FD: %d'ye iletiliyor.\n", conn->info.worker_res.status, conn->info.worker_res.client_fd);

	sfd client_fd = conn->info.worker_res.client_fd;
	struct connection *client_conn = get_connection(client_fd);

	if (client_conn && client_conn->fd != -1) {
		if (conn->body_size > 0) {
			send_all(client_fd, conn->bodybuf, conn->body_size);
		} else {
			char *msg = "OK";
			send_all(client_fd, (i8 *)msg, 2);
		}

		free_connection(client_fd);

	} else {
		printf("HATA: Client fd %d bulunamadı.\n", client_fd);
	}

	reset_connection(worker_fd);
}