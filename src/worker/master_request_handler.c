#include "../../include/worker/master_request_handler.h"

#include "../../include/common/connection.h"
#include "../../include/common/utils.h"
#include "../../include/worker/storage.h"

void handle_master_message(sfd fd) {
	struct connection *conn = get_connection(fd);
	if (!conn) return;

	conn->type = CONN_WOR;
	conn->expected_hdr_size = PRIHDR_SIZE;

	if (conn->state == CONN_WAIT_HEADER) {
		i32 toread = PRIHDR_SIZE - conn->header_bytes_read;
		i32 n = recv(fd, conn->header_buf + conn->header_bytes_read, toread, 0);

		if (n <= 0) {
			if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
			perror("master2worker recv header");
			free_connection(fd);
			return;
		}

		conn->header_bytes_read += n;

		if (conn->header_bytes_read == conn->expected_hdr_size) {
			deserialize_prihdr(&conn->info.master_req, conn->header_buf);

			conn->body_size = conn->info.master_req.pub.keylen + conn->info.master_req.pub.vallen;

			if (conn->body_size > 0) {
				conn->bodybuf = (i8 *)malloc(conn->body_size + 1);
				conn->bodybuf[conn->body_size] = 0;
				conn->body_bytes_read = 0;
				conn->state = CONN_WAIT_PAYLOAD;
			} else {
				// Body yoksa direkt işlem yap
				// düşün burayı
			}
		}
		return;
	}

	if (conn->state == CONN_WAIT_PAYLOAD) {
		i32 toread = conn->body_size - conn->body_bytes_read;
		i32 n = recv(fd, conn->bodybuf + conn->body_bytes_read, toread, 0);

		if (n <= 0) {
			if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
			perror("master2worker recv body");
			free_connection(fd);
			return;
		}

		conn->body_bytes_read += n;

		if (conn->body_bytes_read == conn->body_size) {
			char *raw_key = (char *)conn->bodybuf;
			char *raw_val = (char *)conn->bodybuf + conn->info.master_req.pub.keylen;

			char *key_str = safestrgen(raw_key, conn->info.master_req.pub.keylen);

			struct rethdr hdr;
			hdr.uuid = conn->info.master_req.uuid;
			hdr.client_fd = conn->info.master_req.client_fd;
			i8 buf[RETHDR_SIZE];

			if (conn->info.master_req.pub.op == OP_GET) {
				char *val_found = get(key_str);

				if (val_found != NULL) {
					hdr.status = OK;
					hdr.msglen = strlen(val_found);
					serialize_rethdr(&hdr, buf);
					send_hdr(fd, buf, RETHDR_SIZE);

					send_all(fd, (i8 *)val_found, hdr.msglen);
				} else {
					hdr.status = FAIL;
					hdr.msglen = 0;
					serialize_rethdr(&hdr, buf);
					send_hdr(fd, buf, RETHDR_SIZE);
				}
			}

			if (conn->info.master_req.pub.op == OP_POST) {
				char *val_str = safestrgen(raw_val, conn->info.master_req.pub.vallen);

				STAT res = post(key_str, conn->info.master_req.pub.keylen, val_str, conn->info.master_req.pub.vallen);

				free(val_str);

				hdr.status = res;
				hdr.msglen = 0;

				serialize_rethdr(&hdr, buf);
				send_hdr(fd, buf, RETHDR_SIZE);
			}

			if (conn->info.master_req.pub.op == OP_DEL) {
				del(key_str);

				hdr.status = OK;
				hdr.msglen = 0;

				serialize_rethdr(&hdr, buf);
				send_hdr(fd, buf, RETHDR_SIZE);
			}

			free(key_str);

			reset_connection(fd);

			return;
		}
	}
}