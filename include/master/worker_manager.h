#ifndef MASTER_WORKER_MANAGER_H
#define MASTER_WORKER_MANAGER_H

#include "../../include/common/net.h"
#include "../../include/common/protocol.h"

struct worker {
  sfd sockfd;
  char ip[16];
  char port[6];
  int id;
  int is_connected;
};

void init_workers();

struct worker* get_target_worker(const char* key);

struct worker* get_next_worker(int start_idx);
struct worker* get_worker_by_fd(sfd fd);
struct worker* get_worker_at_index(int index);

void clear_worker(sfd workerfd);
#endif
