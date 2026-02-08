#ifndef MASTER_REQUEST_HANDLER_H
#define MASTER_REQUEST_HANDLER_H

#include "../common/connection.h"
#include "../common/net.h"
#include "worker_manager.h"

void handle_client_message(sfd client_fd);

void handle_worker_message(sfd worker_fd);

#endif