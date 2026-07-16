#include "server.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include "../functions/websocket.h"

#define MAX_CLIENTS 50

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
  // int optlen = sizeof(int);
  // getsockopt(server.sock_fd, SOL_SOCKET, SO_BROADCAST, 1,
  //            (socklen_t *)&optlen);
  if (server.sock_fd < 0) {
    perror("Initialization got shitted on\n");
    exit(1);
  }

  // Set reuse address
  int opt = 1;
  setsockopt(server.sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
    int client_sockets[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) client_sockets[i] = -1;
    
    fd_set read_fds;
    printf("Server running on port %d... Listening for connections.\n", server->port);
    
    while (1) {
        FD_ZERO(&read_fds);
        
        // Add main listening socket to monitor
        FD_SET(server->sock_fd, &read_fds);
        int max_fd = server->sock_fd;
        
        // Add active client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (fd != -1) {
                FD_SET(fd, &read_fds);
                if (fd > max_fd) max_fd = fd;
            }
        }
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            continue;
        }
        
        // 1. New Incoming Connection
        if (FD_ISSET(server->sock_fd, &read_fds)) {
            int addrlen = sizeof(server->address);
            struct sockaddr_in client_address;
            int new_socket = accept(server->sock_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen);
            
            if (new_socket < 0) {
                perror("accept error");
                continue;
            }
            
            // Read standard HTTP request headers
            char buffer[BUFFER_SIZE];
            ssize_t bytesRead = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                
                // If it's a GET request targeting the WebSocket endpoint (/ws)
                if (strstr(buffer, "GET /ws") && strstr(buffer, "Upgrade: websocket")) {
                    if (handle_websocket_handshake(new_socket, buffer) == 0) {
                        // Accept socket persistent
                        int added = 0;
                        for (int i = 0; i < MAX_CLIENTS; i++) {
                            if (client_sockets[i] == -1) {
                                client_sockets[i] = new_socket;
                                added = 1;
                                printf("WS client accepted on socket FD %d\n", new_socket);
                                break;
                            }
                        }
                        if (!added) {
                            printf("Server full. Closing new WS client.\n");
                            close(new_socket);
                        }
                    } else {
                        printf("Handshake failed. Closing connection.\n");
                        close(new_socket);
                    }
                } else {
                    // Serve normal HTTP endpoints (like static GET, POST /login, etc.)
                    response(new_socket, buffer); // response handles closing socket
                }
            } else {
                close(new_socket);
            }
        }
        
        // 2. Active WebSocket Client Events
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_sockets[i];
            if (fd != -1 && FD_ISSET(fd, &read_fds)) {
                char message[BUFFER_SIZE];
                int opcode;
                int len = read_websocket_frame(fd, message, sizeof(message) - 1, &opcode);
                
                if (len <= 0 || opcode == 0x08) {
                    // Connection closed by client or error
                    printf("WS client disconnected on socket FD %d\n", fd);
                    close(fd);
                    client_sockets[i] = -1;
                } else if (opcode == 0x01) {
                    // Broadcast message to all active WebSocket clients in real-time
                    printf("[Broadcast Request from FD %d]: %s\n", fd, message);
                    
                    char broadcast_packet[BUFFER_SIZE + 64];
                    snprintf(broadcast_packet, sizeof(broadcast_packet), "[User on FD %d]: %s", fd, message);
                    
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] != -1) {
                            send_websocket_server_frame(client_sockets[j], broadcast_packet);
                        }
                    }
                }
            }
        }
    }
}
