CC = gcc
CFLAGS = -Wall -Wextra

OBJS = db_operations.o string_operations.o

all: $(OBJS)

db_operations.o: db_operations.c db_operations.h
	$(CC) $(CFLAGS) -c db_operations.c -o db_operations.o

string_operations.o: string_operations.c string_operations.h
	$(CC) $(CFLAGS) -c string_operations.c -o string_operations.o

clean:
	rm -f $(OBJS)

.PHONY: all clean
