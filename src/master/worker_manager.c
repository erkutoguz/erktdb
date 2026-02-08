#include "../../include/master/worker_manager.h"

#include "../../include/common/connection.h"
#include "../../include/common/net.h"
#include "../../include/common/utils.h"

static struct worker workers[WORKER_COUNT];
static char* workerips[WORKER_COUNT] = {"127.0.0.1"};

void init_workers() {
	int n;
	memset(workers, 0, sizeof(workers));

	for (n = 0; n < WORKER_COUNT; ++n) {
		struct worker* w = &workers[n];
		memcpy(w->ip, workerips[n], 16);
		memcpy(w->port, WORKERPORT, 5);
		sfd wfd = connect_to_host(w->ip, WORKERPORT);

		if (wfd == FAIL) {
			workers[n].sockfd = -1;
			continue;
		}
		w->sockfd = wfd;
		struct connection* c = create_connection(wfd);
		if (c) {
			c->type = CONN_WOR;
			c->state = CONN_WAIT_HEADER;
			c->expected_hdr_size = sizeof(struct rethdr);
		} else {
			perror("Worker create_connection failed");
			// burada worker oluşturulazmazsa en başta kapatbiliriz TODO baklılacak
			exit(1);
		}
	}
}

struct worker* get_target_worker(const char* key) {
	i64 idx = hash(key) % WORKER_COUNT;
	return &workers[idx];
}

struct worker* get_worker_at_index(int index) {
	if (index < 0 || index >= WORKER_COUNT) {
		return NULL;
	}
	return &workers[index];
}

struct worker* get_worker_by_fd(sfd fd) {
	for (int i = 0; i < WORKER_COUNT; ++i) {
		if (workers[i].sockfd == fd) {
			return &workers[i];
		}
	}
	return NULL;
}