#ifndef server_h
#define server_h

#include <netinet/in.h>
#include <sqlite3.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

extern sqlite3 *db;

struct Server {
  int domain;
  int port;
  int service;
  int protocol;
  int backlog;
  unsigned long interface;

  int sock_fd;
  struct sockaddr_in address;

  void (*launch)(struct Server *server);
};

struct Server server_constructor(int domain, int port, int service,
                                 int protocol, int backlog,
                                 unsigned long interface,
                                 void (*launch)(struct Server *server));
void launch(struct Server *server);
void response(int new_socket, char *buffer);

#endif