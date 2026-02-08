#ifndef NET_H
#define NET_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "types.h"

#define MAXEVENTS 128
#define MAXCONN 1024
#define WORKERPORT "1999"
#define MASTERPORT "1911"

/**
 * serialize pubhdr struct to buf
 */
void serialize_pubhdr(const struct pubhdr *h, i8 *buf);

/**
 * deserialize buf to pubhdr struct
 */
void deserialize_pubhdr(struct pubhdr *h, const i8 *buf);

/**
 * serialize prihdr struct to buf
 */
void serialize_prihdr(const struct prihdr *h, i8 *buf);

/**
 * deserialize buf to prihdr struct
 */
void deserialize_prihdr(struct prihdr *h, const i8 *buf);

/**
 * serialize pay struct to buf
 */
void serialize_pay(const struct pay *p, i8 *buf, i32 keylen, i32 vallen);

/**
 * deserialize buf to pay struct
 */
void deserialize_pay(struct pay *p, const i8 *buf, i32 keylen, i32 vallen);

void serialize_rethdr(const struct rethdr *h, i8 *buf);

void deserialize_rethdr(struct rethdr *h, const i8 *buf);

/**
 * accept port parameter return listener fd
 */
sfd start_listen(char *port);

/**
 * accept connection coming from listener socket and fill sokcaddr with incoming request data
 * return incoming connection sockfd
 */
sfd accept_conn(int listener, struct sockaddr *addr);

/**
 * master -> worker connection & worker -> master connection
 * returns sockfd
 */
sfd connect_to_host(char *hostname, char *port);

/**
 * send i8 *buff to dest
 * return type status to check if it is success
 */
STAT send_hdr(sfd dest, i8 *buf, i16 size);

/**
 * send payload struct to dest sock
 * pass pay* to send
 * return type status to check if it is success
 */
STAT send_pay(sfd dest, struct pay *pay, i32 keylen, i32 vallen);
/**
 * accept to sfd, i8* buffer and total size in bytes
 * returns OK if it is success
 */
STAT send_all(sfd to, i8 *src, i64 size);

/**
 * accept from sfd, and dest buffer in i8* type and total size in bytes
 * returns OK if it is success
 */
STAT recv_all(sfd from, i8 *dest, i64 size);

/**
 * sets fd to nonblocking
 */
int set_nonblocking(sfd sock);

/**
 * add connected socket to epoll list
 */
STAT add_epoll(sfd epollfd, sfd connfd, struct epoll_event *ev);

#endif