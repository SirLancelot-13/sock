#include "server.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    char str[INET_ADDRSTRLEN];
    printf(" Waiting for Connection...");
    int addrlen = sizeof(server->address);
    struct sockaddr_in client_address;
    int new_socket = accept(server->sock_fd, (struct sockaddr *)&client_address,
                            (socklen_t *)&addrlen);
    ssize_t bytesRead = read(new_socket, buffer, BUFFER_SIZE - 1);
    if (bytesRead >= 0) {
      buffer[bytesRead] = '\0';
      puts(buffer);
    } else {
      perror("Couldn't read buffer.");
    }
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    if (getpeername(new_socket, (struct sockaddr *)&peer_addr,
                    &peer_addr_len) == 0) {
      char peer_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), peer_ip,
                INET_ADDRSTRLEN);
      printf("IP Address of the peer: %s\n\n", peer_ip);
    }
    // Using the new_socket here as it is non-blocking.
    char *response = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                     "<!DOCTYPE html>\r\n"
                     "<html>\r\n"
                     "<head>\r\n"
                     "<title>Testing Basic HTTP-SERVER</title>\r\n"
                     "</head>\r\n"
                     "<body>\r\n"
                     "Hello, World!\r\n"
                     "</body>\r\n"
                     "</html>\r\n";
    write(new_socket, response, strlen(response));
    close(new_socket);
  }
}
