#ifndef MASTER_WORKER_MANAGER_H
#define MASTER_WORKER_MANAGER_H

#include "../../include/common/net.h"
#include "../../include/common/protocol.h"

#define WORKER_COUNT 2
#define wip1 "192.168.85.233"
#define wip2 "127.0.0.1"

struct worker {
	sfd sockfd;	   // Worker ile konuşulan soket
	char ip[16];	   // Worker IP adresi
	char port[6];	   // Worker Portu
	int id;		   // 0, 1, 2...
	int is_connected;  // 1: Bağlı, 0: Kopuk
};

/**
 * Config'deki veya kodda gömülü IP'leri kullanarak
 * tüm workerlara connect_to_host() yapar.
 * Başarılı olanları workers[] dizisine kaydeder.
 */
void init_workers();

/**
 * Anahtarın (key) hash değerine göre hedef worker'ı seçer.
 * Yük dengeleme (Load Balancing) burada yapılır.
 * Örn: hash(key) % WORKER_COUNT
 */
struct worker* get_target_worker(const char* key);

/**
 * Bir Worker'ın soket numarasını (fd) verince, onun struct yapısını döner.
 * Worker'dan cevap geldiğinde "Bu cevabı kim yolladı?" demek için kullanılır.
 */
struct worker* get_worker_by_fd(sfd fd);

#endif