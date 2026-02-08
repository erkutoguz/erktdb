#include <netinet/in.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "../../include/common/connection.h"
#include "../../include/master/request_handler.h"
#include "../../include/master/worker_manager.h"

// worker connection kapandığında master kapanmamalı onu da kurmalıyız.
// eğer hiç bir worker kalmaz ise sistem client'e doğrudan hata mesajı yollasın kapanmasın

int main() {
	struct epoll_event events[MAXEVENTS], ev;
	struct worker *w;
	struct connection *c;
	sfd epollfd, connsock;
	int nfds, n;

	struct sockaddr incomaddr;
	char incomip[16];

	init_connections();
	init_workers();

	sfd serv = start_listen(MASTERPORT);

	if (set_nonblocking(serv) == FAIL) {
		perror("setnonblock");
		exit(EXIT_FAILURE);
	}

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epollcreate");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = serv;

	if (add_epoll(epollfd, serv, &ev) == FAIL) {
		perror("addepoll");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < WORKER_COUNT; ++i) {
		struct worker *w = get_worker_at_index(i);

		if (w && w->sockfd != -1) {
			struct epoll_event ev;
			ev.events = EPOLLIN;
			ev.data.fd = w->sockfd;
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, w->sockfd, &ev) == -1) {
				perror("epoll_ctl: worker socket");
				exit(EXIT_FAILURE);
			}
		}
	}

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
		if (nfds == -1) {
			perror("epollwait");
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {
			int current_fd = events[n].data.fd;

			if (current_fd == serv) {
				if ((connsock = accept_conn(serv, &incomaddr)) == FAIL) {
					exit(EXIT_FAILURE);
				}

				set_nonblocking(connsock);

				if (add_epoll(epollfd, connsock, &ev) == FAIL) {
					perror("addepoll connsock");

					exit(EXIT_FAILURE);
				}
				create_connection(connsock);
				inet_ntop(incomaddr.sa_family, &(((struct sockaddr_in *)&incomaddr)->sin_addr), incomip, INET_ADDRSTRLEN);
				printf("connection initialized...\nip: %s\n", incomip);

				continue;
			}

			struct worker *w = get_worker_by_fd(current_fd);
			if (w) {
				handle_worker_message(current_fd);
				continue;
			}

			struct connection *c = get_connection(current_fd);
			if (c) {
				handle_client_message(current_fd);
			} else {
				epoll_ctl(epollfd, EPOLL_CTL_DEL, current_fd, NULL);
				close(current_fd);
			}
		}
	}
	return 0;
}