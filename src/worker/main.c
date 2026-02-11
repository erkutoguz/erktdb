#include <signal.h>

#include "../../include/common/connection.h"
#include "../../include/common/net.h"
#include "../../include/worker/master_request_handler.h"
#include "../../include/worker/storage.h"

char dbfilename[12];
char imbackupdbfilename[12];

int backupfd = -1;

void handle_sigint(int sig) {
  printf("\nworker shutdown\n");
  save(dbfilename, 0);
  save(imbackupdbfilename, 1);
  exit(0);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("usage: %s <id> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int myid = atoi(argv[1]);
  char* myport = argv[2];
  signal(SIGINT, handle_sigint);
  int backupid = (myid + 1) % WORKERCOUNT;
  char* backupip = wips[backupid];
  char* backupport = wports[backupid];

  int imbackup = (myid - 1 + WORKERCOUNT) % WORKERCOUNT;
  char* imbackupport = wports[imbackup];

  int nretry = 0;

  sfd listener = start_listen(myport);

  if (backupid != myid) {
    while (backupfd < 0 && nretry < 3) {
      backupfd = worker2worker(backupip, backupport);

      if (backupfd < 0) {
        sleep(2);
        nretry++;
      }
    }
    if (backupfd > 0) {
      printf("connected: worker %d -> worker %d\n", myid, backupid);
    } else {
      printf("cannot connect to worker %d port %s\n", backupid, backupport);
      backupfd = -1;
    }
  }

  sprintf(dbfilename, "%s.db", myport);
  sprintf(imbackupdbfilename, "%s.db.bc", imbackupport);

  load(dbfilename, 0);
  load(imbackupdbfilename, 1);

  struct epoll_event events[MAXEVENTS], ev;
  struct sockaddr incomaddr;

  sfd epollfd, connsock;
  int nfds, n;

  char incomip[16];

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

  if (backupfd > 0) {
    set_nonblocking(backupfd);

    create_connection(backupfd);

    struct epoll_event backup_ev;
    backup_ev.events = EPOLLIN;
    backup_ev.data.fd = backupfd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, backupfd, &backup_ev) == -1) {
      perror("epoll add backupfd");
    } else {
      struct prihdr req;
      memset(&req, 0, sizeof(req));
      req.pub.op = OP_SYNC_START;
      req.flag = 0;

      i8 buf[PRIHDR_SIZE];
      serialize_prihdr(&req, buf);
      send_hdr(backupfd, buf, PRIHDR_SIZE);
    }
  }

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

        inet_ntop(incomaddr.sa_family,
                  &(((struct sockaddr_in*)&incomaddr)->sin_addr), incomip,
                  INET_ADDRSTRLEN);
        printf("connection initialized...\nip: %s\n", incomip);
        continue;
      }

      struct connection* c = get_connection(current_fd);
      if (c) {
        handle_master_message(current_fd);
      } else {
        close(current_fd);
      }
    }
  }
  return 0;
}
