CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

FUNC_DIR = ../functions
TARGET = client_bin
LIB_TARGET = libsockclient.a

OBJS = client.o
FUNC_OBJS = $(FUNC_DIR)/string_operations.o $(FUNC_DIR)/crypt.o $(FUNC_DIR)/websocket.o

all: func_objs $(TARGET) lib

func_objs:
	$(MAKE) -C $(FUNC_DIR) string_operations.o crypt.o websocket.o

$(TARGET): $(OBJS) $(FUNC_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(FUNC_OBJS) -o $(TARGET) $(LDFLAGS)

lib: $(LIB_TARGET)

$(LIB_TARGET): $(OBJS) $(FUNC_OBJS)
	ar rcs $(LIB_TARGET) $(OBJS) $(FUNC_OBJS)

client.o: client.c client.h ../server/server.h $(FUNC_DIR)/string_operations.h $(FUNC_DIR)/db_operations.h $(FUNC_DIR)/websocket.h
	$(CC) $(CFLAGS) -c client.c -o client.o

clean:
	rm -f $(OBJS) $(TARGET) $(LIB_TARGET)
	$(MAKE) -C $(FUNC_DIR) clean

.PHONY: all clean func_objs lib

