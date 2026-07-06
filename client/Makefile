CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

FUNC_DIR = ../functions
TARGET = client_bin

OBJS = client.o
FUNC_OBJS = $(FUNC_DIR)/string_operations.o

all: func_objs $(TARGET)

func_objs:
	$(MAKE) -C $(FUNC_DIR) string_operations.o

$(TARGET): $(OBJS) $(FUNC_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(FUNC_OBJS) -o $(TARGET) $(LDFLAGS)

client.o: client.c client.h ../server/server.h $(FUNC_DIR)/string_operations.h $(FUNC_DIR)/db_operations.h
	$(CC) $(CFLAGS) -c client.c -o client.o

clean:
	rm -f $(OBJS) $(TARGET)
	$(MAKE) -C $(FUNC_DIR) clean

.PHONY: all clean func_objs
