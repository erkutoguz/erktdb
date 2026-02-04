#ifndef MASTER_REQUEST_HANDLER_H
#define MASTER_REQUEST_HANDLER_H

#include "../common/connection.h"
#include "../common/net.h"

void handle_master_message(sfd fd);

STAT send_master_stat(sfd fd, STAT stat);

#endif