#ifndef MASTER_CONNECTION_H
#define MASTER_CONNECTION_H

#include "../../include/common/net.h"
#include "../../include/common/protocol.h"

#define MAX_CLIENTS 1024

typedef enum { CONN_WAIT_HEADER, CONN_WAIT_PAYLOAD, CONN_IDLE } conn_state;

struct connection {
	sfd fd;
	sfd workerfd;
	conn_state state;

	i8 header_buf[12];
	i32 header_bytes_read;

	struct hdr header;
	i8* payload_buf;
	i32 payload_size;
	i32 payload_bytes_read;
};

/**
 * Global connection dizisini sıfırlar (memset).
 * Main'in en başında çağrılır.
 */
void init_connections();

/**
 * Yeni bağlanan bir client (accept_conn sonrası) için boş bir slot bulur.
 * fd'yi kaydeder ve state'i WAIT_HEADER yapar.
 * Hata durumunda NULL döner.
 */
struct connection* create_connection(sfd fd);

/**
 * Soket numarası (fd) verilen bağlantının struct pointer'ını getirir.
 * O(1) hızında erişim için genelde connections[fd] kullanılır.
 */
struct connection* get_connection(sfd fd);

void clear_connection(sfd fd);

/**
 * Client koptuğunda çağrılır.
 * Varsa payload_buf'ı free eder ve slotu temizler.
 */
void free_connection(sfd fd);

#endif