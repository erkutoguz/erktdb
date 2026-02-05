# Compiler ve Bayraklar
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

# Klasörler
SRC_DIR = src
BUILD_DIR = build

# ------------------------------------------------
# ÇIKTI DOSYALARI (Root dizinde kalacaklar)
# ------------------------------------------------
MASTER_BIN = master_node
WORKER_BIN = worker_node
CLIENT_BIN = client_app

# ------------------------------------------------
# KAYNAK DOSYALAR
# ------------------------------------------------
COMMON_SRC = src/common/connection.c src/common/net.c src/common/utils.c
MASTER_SRC = src/master/main.c src/master/request_handler.c src/master/worker_manager.c
WORKER_SRC = src/worker/main.c src/worker/master_request_handler.c src/worker/storage.c
CLIENT_SRC = test/client.c

# ------------------------------------------------
# OBJECT DOSYALARI (Hepsi build/ altına gidecek)
# ------------------------------------------------
# Mantık: src/master/main.c  ->  build/src/master/main.o
COMMON_OBJ = $(COMMON_SRC:%.c=$(BUILD_DIR)/%.o)
MASTER_OBJ = $(MASTER_SRC:%.c=$(BUILD_DIR)/%.o)
WORKER_OBJ = $(WORKER_SRC:%.c=$(BUILD_DIR)/%.o)
CLIENT_OBJ = $(CLIENT_SRC:%.c=$(BUILD_DIR)/%.o)

# Varsayılan hedef
all: master worker client

# ----------------------------------------
# LINKING (Bağlama Aşaması - Binary Üretimi)
# Çıktılar (-o) direkt ismin kendisine (root'a) gidiyor.
# ----------------------------------------

master: $(MASTER_OBJ) $(COMMON_OBJ)
	@echo "🔗 Master Node kök dizine bağlanıyor..."
	$(CC) $(CFLAGS) -o $(MASTER_BIN) $(MASTER_OBJ) $(COMMON_OBJ)
	@echo "✅ Master Node Hazır: ./$(MASTER_BIN)"

worker: $(WORKER_OBJ) $(COMMON_OBJ)
	@echo "🔗 Worker Node kök dizine bağlanıyor..."
	$(CC) $(CFLAGS) -o $(WORKER_BIN) $(WORKER_OBJ) $(COMMON_OBJ)
	@echo "✅ Worker Node Hazır: ./$(WORKER_BIN)"

client: $(CLIENT_OBJ) $(COMMON_OBJ)
	@echo "🔗 Test Client kök dizine bağlanıyor..."
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_OBJ) $(COMMON_OBJ)
	@echo "✅ Client Hazır: ./$(CLIENT_BIN)"

# ----------------------------------------
# COMPILING (Derleme Aşaması - Object Üretimi)
# Sadece burası build/ klasörünü kullanır
# ----------------------------------------

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "🔨 Derleniyor: $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------------------
# TEMİZLİK
# ----------------------------------------
clean:
	@echo "🧹 Temizlik yapılıyor..."
	# Sadece build klasörünü ve kökteki exe'leri sil
	rm -rf $(BUILD_DIR)
	rm -f $(MASTER_BIN) $(WORKER_BIN) $(CLIENT_BIN)
	@echo "✨ build/ klasörü ve binaryler temizlendi."

.PHONY: all master