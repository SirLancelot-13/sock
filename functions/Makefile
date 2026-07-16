CC = gcc
CFLAGS = -Wall -Wextra

OBJS = db_operations.o string_operations.o crypt.o websocket.o

all: $(OBJS)

db_operations.o: db_operations.c db_operations.h
	$(CC) $(CFLAGS) -c db_operations.c -o db_operations.o

string_operations.o: string_operations.c string_operations.h
	$(CC) $(CFLAGS) -c string_operations.c -o string_operations.o

crypt.o: crypt.c crypt.h
	$(CC) $(CFLAGS) -c crypt.c -o crypt.o

websocket.o: websocket.c websocket.h crypt.h
	$(CC) $(CFLAGS) -c websocket.c -o websocket.o

clean:
	rm -f $(OBJS)

.PHONY: all clean
