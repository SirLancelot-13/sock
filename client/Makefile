CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lsqlite3

FUNC_DIR = ../functions
TARGET = client_bin

OBJS = client.o
FUNC_OBJS = $(FUNC_DIR)/db_operations.o $(FUNC_DIR)/string_operations.o

all: func_objs $(TARGET)

func_objs:
	$(MAKE) -C $(FUNC_DIR)

$(TARGET): $(OBJS) $(FUNC_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(FUNC_OBJS) -o $(TARGET) $(LDFLAGS)

client.o: client.c client.h ../server/server.h $(FUNC_DIR)/string_operations.h $(FUNC_DIR)/db_operations.h
	$(CC) $(CFLAGS) -c client.c -o client.o

clean:
	rm -f $(OBJS) $(TARGET)
	$(MAKE) -C $(FUNC_DIR) clean

.PHONY: all clean func_objs
