#ifndef MASTER_REQUEST_HANDLER_H
#define MASTER_REQUEST_HANDLER_H

#include "../common/connection.h"
#include "../common/net.h"
#include "worker_manager.h"

void handle_client_message(sfd client_fd);

/**
 * SENARYO C: WORKER CEVAP DÖNDÜ
 * * Epoll, bir Worker fd'sinde EPOLLIN verirse bu çağrılır.
 * 1. Worker'dan gelen cevabı (OK/FAIL + Value) okur.
 * 2. Bu cevabın asıl sahibi olan Client'a iletir.
 */
void handle_worker_message(sfd worker_fd);

#endif