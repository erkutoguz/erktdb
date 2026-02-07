#include "../../include/common/net.h"

void serialize_pubhdr(const struct pubhdr *h, i8 *buf) {
	int offset = 0;
	buf[offset++] = (h->magic >> 8) & 0xff;
	buf[offset++] = h->magic & 0xff;

	buf[offset++] = h->op & 0xff;
	buf[offset++] = 0x00;

	buf[offset++] = (h->keylen >> 24) & 0xff;
	buf[offset++] = (h->keylen >> 16) & 0xff;
	buf[offset++] = (h->keylen >> 8) & 0xff;
	buf[offset++] = h->keylen & 0xff;

	buf[offset++] = (h->vallen >> 24) & 0xff;
	buf[offset++] = (h->vallen >> 16) & 0xff;
	buf[offset++] = (h->vallen >> 8) & 0xff;
	buf[offset++] = h->vallen & 0xff;
}

void deserialize_pubhdr(struct pubhdr *h, const i8 *buf) {
	h->magic = (buf[0] << 8) | (buf[1]);

	h->op = buf[2];
	h->pad = 0;

	h->keylen = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | (buf[7]);
	h->vallen = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11];
}

void serialize_prihdr(const struct prihdr *h, i8 *buf) {
	int offset = 0;
	buf[offset++] = (h->pub.magic >> 8) & 0xff;
	buf[offset++] = h->pub.magic & 0xff;

	buf[offset++] = h->pub.op & 0xff;
	buf[offset++] = 0x00;

	buf[offset++] = (h->pub.keylen >> 24) & 0xff;
	buf[offset++] = (h->pub.keylen >> 16) & 0xff;
	buf[offset++] = (h->pub.keylen >> 8) & 0xff;
	buf[offset++] = h->pub.keylen & 0xff;

	buf[offset++] = (h->pub.vallen >> 24) & 0xff;
	buf[offset++] = (h->pub.vallen >> 16) & 0xff;
	buf[offset++] = (h->pub.vallen >> 8) & 0xff;
	buf[offset++] = h->pub.vallen & 0xff;

	buf[offset++] = (h->client_fd >> 24) & 0xff;
	buf[offset++] = (h->client_fd >> 16) & 0xff;
	buf[offset++] = (h->client_fd >> 8) & 0xff;
	buf[offset++] = (h->client_fd) & 0xff;

	buf[offset++] = (h->uuid >> 56) & 0xff;
	buf[offset++] = (h->uuid >> 48) & 0xff;
	buf[offset++] = (h->uuid >> 40) & 0xff;
	buf[offset++] = (h->uuid >> 32) & 0xff;
	buf[offset++] = (h->uuid >> 24) & 0xff;
	buf[offset++] = (h->uuid >> 16) & 0xff;
	buf[offset++] = (h->uuid >> 8) & 0xff;
	buf[offset++] = h->uuid & 0xff;
}

void deserialize_prihdr(struct prihdr *h, const i8 *buf) {
	h->pub.magic = (buf[0] << 8) | (buf[1]);

	h->pub.op = buf[2];
	h->pub.pad = 0;

	h->pub.keylen = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | (buf[7]);
	h->pub.vallen = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11];

	h->client_fd = (buf[12] << 24) | (buf[13] << 16) | (buf[14] << 8) | buf[15];
	h->uuid = ((i64)buf[16] << 56) | ((i64)buf[17] << 48) | ((i64)buf[18] << 40) | ((i64)buf[19] << 32) | (buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | (buf[23]);
}

void serialize_pay(const struct pay *p, i8 *buf, i32 keylen, i32 vallen) {
	i32 i, offset = 0;
	i8 *key = p->key;
	i8 *val = p->val;

	for (i = 0; i < keylen; ++i) {
		buf[offset++] = key[i];
	}

	for (i = 0; i < vallen; ++i) {
		buf[offset++] = val[i];
	}
}

void deserialize_pay(struct pay *p, const i8 *buf, i32 keylen, i32 vallen) {
	i32 i, offset = 0;
	for (i = 0; i < keylen; ++i) {
		p->key[i] = buf[offset++];
	}
	for (i = 0; i < vallen; ++i) {
		p->val[i] = buf[offset++];
	}
}

sfd start_listen(char *port) {
	struct addrinfo hints, *addr, *p;
	sfd listener;
	int stat, yes = 1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((stat = getaddrinfo(NULL, port, &hints, &addr)) == -1) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(stat));
		exit(EXIT_FAILURE);
	}

	for (p = addr; p != NULL; p = p->ai_next) {
		if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			perror("socket");
			continue;
		}

		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}

		if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
			perror("bind");
			continue;
		}
		break;
	}

	if (listen(listener, 128) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(addr);

	return listener;
}

sfd connect_to_host(char *hostname, char *port) {
	struct addrinfo hints, *addr, *p;
	sfd hostfd;
	int stat;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((stat = getaddrinfo(hostname, port, &hints, &addr)) == -1) {
		fprintf(stderr, "connect2host getaddrinfo %s\n", gai_strerror(stat));
		return FAIL;
	}

	for (p = addr; p != NULL; p = p->ai_next) {
		if ((hostfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			perror("connect2host socket");
			continue;
		}

		if (connect(hostfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("connect2host connect");
			close(hostfd);
			continue;
		}

		break;
	}
	if (p == NULL) {
		fprintf(stderr, "connect2host\n");
		freeaddrinfo(addr);
		return FAIL;
	}

	freeaddrinfo(addr);
	return hostfd;
}

sfd accept_conn(int listener, struct sockaddr *addr) {
	sfd connfd;
	socklen_t size = sizeof(struct sockaddr);

	if ((connfd = accept(listener, addr, &size)) == -1) {
		perror("acceptconn");
		return FAIL;
	}
	return connfd;
}

STAT send_hdr(sfd dest, i8 *buf, i16 size) { return send_all(dest, buf, size) == OK ? OK : FAIL; }

STAT send_pay(sfd dest, struct pay *pay, i32 keylen, i32 vallen) {
	i8 *buf;
	buf = (i8 *)malloc(keylen + vallen);
	i64 size = keylen + vallen;

	serialize_pay(pay, buf, keylen, vallen);
	STAT stat = send_all(dest, buf, size);
	free(buf);
	return stat == OK ? OK : FAIL;
}

STAT send_all(sfd to, i8 *src, i64 size) {
	i64 tot_sent = 0;
	i64 sent;

	while (tot_sent < size) {
		sent = send(to, src + tot_sent, size - tot_sent, 0);

		if (sent == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				usleep(1000);
				continue;
			}
			perror("sendall");
			return FAIL;
		}
		tot_sent += sent;
	}
	return OK;
}

STAT recv_all(sfd from, i8 *dst, i64 size) {
	i64 tot_recv = 0;
	i64 recvd;

	while (tot_recv < size) {
		recvd = recv(from, dst + tot_recv, size - tot_recv, 0);
		if (recvd == -1) {
			perror("recvall");
			return FAIL;
		}
		tot_recv += recvd;
	}
	return OK;
}

int set_nonblocking(sfd sock) {
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) return FAIL;
	flags |= O_NONBLOCK;
	return fcntl(sock, F_SETFL, flags);
}

STAT add_epoll(sfd epollfd, sfd connfd, struct epoll_event *ev) {
	ev->events = EPOLLIN;
	ev->data.fd = connfd;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, ev) == -1) {
		perror("epollctl addepoll");
		return FAIL;
	}
	return OK;
}