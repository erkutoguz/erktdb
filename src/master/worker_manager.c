#include "../../include/master/worker_manager.h"

#include "../../include/common/net.h"

static struct worker workers[WORKER_COUNT];
static char* workerips[WORKER_COUNT] = {"192.168.85.233"};

void init_workers() {
	int n;
	memset(workers, 0, sizeof(workers));

	for (n = 0; n < WORKER_COUNT; ++n) {
		struct worker* w = &workers[n];
		memcpy(w->ip, workerips[n], 16);
		memcpy(w->port, WORKERPORT, 5);
		sfd wfd = connect_to_host(w->ip, WORKERPORT);
		// if (wfd == FAIL) exit(EXIT_FAILURE);
		if (wfd == FAIL) {
			workers[n].sockfd = -1;
			continue;
		}
		w->sockfd = wfd;
	}
}

struct worker* get_target_worker() { return &workers[0]; }

/**
 * Bir Worker'ın soket numarasını (fd) verince, onun struct yapısını döner.
 * Worker'dan cevap geldiğinde "Bu cevabı kim yolladı?" demek için kullanılır.
 */
struct worker* get_worker_by_fd(sfd fd) {
	if (fd < 0 || fd > WORKER_COUNT) return NULL;
	if (workers[fd].sockfd == -1) return NULL;
	return &workers[fd];
}