#include <signal.h>

#include "../../include/common/connection.h"
#include "../../include/common/net.h"
#include "../../include/worker/master_request_handler.h"
#include "../../include/worker/storage.h"

void handle_sigint(int sig) {
	printf("\nworker shutdown\n");
	save("erkt.db");
	exit(0);
}

int main() {
	signal(SIGINT, handle_sigint);
	struct epoll_event events[MAXEVENTS], ev;
	struct sockaddr incomaddr;

	sfd epollfd, connsock;
	int nfds, n;

	char incomip[16];

	load("erkt.db");
	sfd listener = start_listen(WORKERPORT);
	if (set_nonblocking(listener) == FAIL) {
		perror("setnonblock");
		exit(EXIT_FAILURE);
	}

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epollcreate");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listener;

	if (add_epoll(epollfd, listener, &ev) == FAIL) {
		perror("addepoll");
		exit(EXIT_FAILURE);
	}

	i32 bytes_read = 0;
	i32 toread = 26;

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
		if (nfds == -1) {
			perror("epollwait");
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {
			sfd current_fd = events[n].data.fd;

			if (current_fd == listener) {
				if ((connsock = accept_conn(listener, &incomaddr)) == FAIL) {
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

			struct connection *c = get_connection(current_fd);
			if (c) {
				handle_master_message(current_fd);
			} else {
				close(current_fd);
			}
		}
	}
	return 0;
}