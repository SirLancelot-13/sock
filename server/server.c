#include "server.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

struct Server server_constructor(int domain, int port, int service,
                                 int protocol, int backlog,
                                 unsigned long interface,
                                 void (*launch)(struct Server *server)) {
  struct Server server;
  server.domain = domain;
  server.port = port;
  server.service = service;
  server.protocol = protocol;
  server.backlog = backlog;
  server.address.sin_family = domain;
  server.address.sin_port = htons(port);
  server.address.sin_addr.s_addr = htonl(interface);

  server.sock_fd = socket(domain, service, protocol);

  if (server.sock_fd < 0) {
    perror("Initialization got shitted on\n");
    exit(1);
  }

  if (bind(server.sock_fd, (struct sockaddr *)&server.address,
           sizeof(server.address)) < 0) {
    perror("Binding to socket failed gng/n");
    exit(1);
  }

  if (listen(server.sock_fd, server.backlog) < 0) {
    perror("Failed to start listening \n");
    exit(1);
  }

  server.launch = launch;
  return server;
}

void launch(struct Server *server) {
  char buffer[BUFFER_SIZE];
  while (1) {
    printf(" Waiting for Connection...");
    int addrlen = sizeof(server->address);
    struct sockaddr_in client_address;
    int new_socket = accept(server->sock_fd, (struct sockaddr *)&client_address,
                            (socklen_t *)&addrlen);
    ssize_t bytesRead = read(new_socket, buffer, BUFFER_SIZE - 1);
    if (bytesRead >= 0) {
      buffer[bytesRead] = '\0';
      puts(buffer);
      response(new_socket, buffer);
    } else {
      perror("Couldn't read buffer.");
      close(new_socket);
    }
  }
}
