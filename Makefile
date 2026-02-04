CC = gcc

CFLAGS = -Wall -Wextra -g -I./include

TARGET_MASTER = master_node

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin


SRCS_COMMON = $(wildcard $(SRC_DIR)/common/*.c)
SRCS_MASTER = $(wildcard $(SRC_DIR)/master/*.c)

OBJS_COMMON = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_COMMON))
OBJS_MASTER = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_MASTER))

all: directories $(TARGET_MASTER)


$(TARGET_MASTER): $(OBJS_COMMON) $(OBJS_MASTER)
	@echo "Linking Master Node..."
	$(CC) $(CFLAGS) $^ -o $@
	@echo "Basariyla olusturuldu: $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

directories:
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(TARGET_MASTER)
	@echo "Clean done."

.PHONY: all clean directories