#ifndef MASTER_WORKER_MANAGER_H
#define MASTER_WORKER_MANAGER_H

#include "../../include/common/net.h"
#include "../../include/common/protocol.h"

#define WORKER_COUNT 1
#define wip1 "192.168.85.233"
#define wip2 "127.0.0.1"

struct worker {
	sfd sockfd;
	char ip[16];
	char port[6];
	int id;
	int is_connected;
};

void init_workers();

struct worker* get_target_worker(const char* key);

struct worker* get_worker_by_fd(sfd fd);

struct worker* get_worker_at_index(int index);

#endif