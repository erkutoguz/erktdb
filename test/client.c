#include "../include/common/net.h"

int main() {
	struct addrinfo hints, *addr;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, "1911", &hints, &addr);

	int fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

	connect(fd, addr->ai_addr, addr->ai_addrlen);

	struct hdr h;
	h.magic = 0x0000;
	h.op = OP_GET;
	h.keylen = (i32)4;
	h.vallen = (i32)6;

	struct pay p;
	// Malloc ile uğraşmadan direkt string literal verebilirsin test için
	p.key = "name";
	p.val = "zeynep";

	i8 buf[12];
	i8 pbuf[10];  // 4 + 5

	serialize_hdr(&h, buf);

	// serialize_pay zaten p.key ve p.val'i okuyup pbuf'a yazar.
	// memcpy yapmana gerek yok.
	serialize_pay(&p, pbuf, h.keylen, h.vallen);

	send_all(fd, buf, 12);
	send_all(fd, pbuf, 10);

	printf("Veri gönderildi, kapanıyor...\n");
	printf("hdr size %lu\n", sizeof(struct hdr));
	close(fd);
	freeaddrinfo(addr);
	return 0;
}