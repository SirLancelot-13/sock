CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

FUNC_DIR = ../functions
LIB_TARGET = libsockclient.a

OBJS = client.o
FUNC_OBJS = $(FUNC_DIR)/string_operations.o $(FUNC_DIR)/crypt.o $(FUNC_DIR)/websocket.o

all: func_objs lib

func_objs:
	$(MAKE) -C $(FUNC_DIR) string_operations.o crypt.o websocket.o

client.o: client.c client.h
	$(CC) $(CFLAGS) -c client.c -o client.o

lib: $(LIB_TARGET)

$(LIB_TARGET): $(OBJS) $(FUNC_OBJS)
	ar rcs $(LIB_TARGET) $(OBJS) $(FUNC_OBJS)

clean:
	rm -f $(OBJS) $(LIB_TARGET)
	$(MAKE) -C $(FUNC_DIR) clean

.PHONY: all clean func_objs lib
