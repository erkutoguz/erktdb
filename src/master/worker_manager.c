#include "../../include/master/worker_manager.h"

#include "../../include/common/connection.h"
#include "../../include/common/net.h"
#include "../../include/common/utils.h"

static struct worker workers[WORKERCOUNT];

void init_workers() {
  int n;
  memset(workers, 0, sizeof(workers));

  for (n = 0; n < WORKERCOUNT; ++n) {
    struct worker* w = &workers[n];
    w->id = n;

    memcpy(w->ip, wips[n], strlen(wips[n]));
    memcpy(w->port, wports[n], 5);

    sfd wfd = connect_to_host(w->ip, wports[n]);

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
  for (int n = 1; n < WORKERCOUNT; ++n) {
    int idx = (start_idx + n) % WORKERCOUNT;

    if (workers[idx].is_connected == 1 && workers[idx].sockfd != -1) {
      return &workers[idx];
    }
  }
  return NULL;
}

struct worker* get_target_worker(const char* key) {
  i64 idx = hash(key) % WORKERCOUNT;
  // struct worker* w = &workers[idx];
  // if (w->is_connected == 0 || w->sockfd == -1) {
  // 	printf("target worker %ld dead. searching for alive...\n", idx);
  // 	return get_next_worker(idx);
  // }
  return &workers[idx];
}

struct worker* get_worker_at_index(int index) {
  if (index < 0 || index >= WORKERCOUNT) {
    return NULL;
  }

  return &workers[index];
}

struct worker* get_worker_by_fd(sfd fd) {
  for (int n = 0; n < WORKERCOUNT; ++n) {
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
