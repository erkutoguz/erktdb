#include "../../include/master/worker_manager.h"

#include "../../include/common/connection.h"
#include "../../include/common/net.h"
#include "../../include/common/utils.h"

static struct worker workers[WORKER_COUNT];
static char* workerips[WORKER_COUNT] = {"127.0.0.1", "192.168.1.5"};

void init_workers() {
	int n;
	memset(workers, 0, sizeof(workers));

	for (n = 0; n < WORKER_COUNT; ++n) {
		struct worker* w = &workers[n];
		w->id = n;

		memcpy(w->ip, workerips[n], strlen(workerips[n]));
		memcpy(w->port, WORKERPORT, 5);

		sfd wfd = connect_to_host(w->ip, WORKERPORT);

		if (wfd == FAIL) {
			workers[n].sockfd = -1;
			workers[n].is_connected = 0;
			printf("worker %d couldnt conn\n", n);
			continue;
		}
		w->sockfd = wfd;
		w->is_connected = 1;

		struct connection* c = create_connection(wfd);
		if (c) {
			c->type = CONN_WOR;
			c->state = CONN_WAIT_HEADER;
			c->expected_hdr_size = RETHDR_SIZE;
		} else {
			w->is_connected = 0;
			w->sockfd = -1;
			perror("Worker create_connection failed");
		}
	}
}

struct worker* get_next_worker(int start_idx) {
	for (int n = 1; n < WORKER_COUNT; ++n) {
		int idx = (start_idx + n) % WORKER_COUNT;

		if (workers[idx].is_connected == 1 && workers[idx].sockfd != -1) {
			return &workers[idx];
		}
	}
	return NULL;
}

struct worker* get_target_worker(const char* key) {
	i64 idx = hash(key) % WORKER_COUNT;
	struct worker* w = &workers[idx];
	if (w->is_connected == 0 || w->sockfd == -1) {
		printf("target worker %ld dead. searching for alive...\n", idx);
		return get_next_worker(idx);
	}
	return &workers[idx];
}

struct worker* get_worker_at_index(int index) {
	if (index < 0 || index >= WORKER_COUNT) {
		return NULL;
	}

	return &workers[index];
}

struct worker* get_worker_by_fd(sfd fd) {
	for (int n = 0; n < WORKER_COUNT; ++n) {
		if (workers[n].sockfd == fd) {
			return &workers[n];
		}
	}
	return NULL;
}

void clear_worker(sfd workerfd) {
	struct worker* w = get_worker_by_fd(workerfd);
	w->is_connected = 0;
	w->sockfd = -1;
}