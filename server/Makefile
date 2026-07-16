CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lsqlite3

FUNC_DIR = ../functions
TARGET = server_bin

OBJS = main.o server.o endpoints.o
FUNC_OBJS = $(FUNC_DIR)/db_operations.o $(FUNC_DIR)/crypt.o $(FUNC_DIR)/websocket.o

all: func_objs $(TARGET)

func_objs:
	$(MAKE) -C $(FUNC_DIR) db_operations.o crypt.o websocket.o

$(TARGET): $(OBJS) $(FUNC_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(FUNC_OBJS) -o $(TARGET) $(LDFLAGS)

main.o: main.c server.h $(FUNC_DIR)/db_operations.h
	$(CC) $(CFLAGS) -c main.c -o main.o

server.o: server.c server.h $(FUNC_DIR)/websocket.h
	$(CC) $(CFLAGS) -c server.c -o server.o

# endpoints.c directly #includes ../functions/string_operations.c,
# so string_operations.o must NOT be linked separately.
endpoints.o: endpoints.c server.h $(FUNC_DIR)/string_operations.c $(FUNC_DIR)/string_operations.h $(FUNC_DIR)/db_operations.h
	$(CC) $(CFLAGS) -c endpoints.c -o endpoints.o

clean:
	rm -f $(OBJS) $(TARGET)
	$(MAKE) -C $(FUNC_DIR) clean

.PHONY: all clean func_objs
