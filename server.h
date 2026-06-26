#ifndef server_h
#define server_h

#include <netinet/in.h>

#define BUFFER_SIZE 1024

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

struct Server server_constructor(int domain, int port, int service, int protocol, int backlog, unsigned long interface,void (*launch)(struct Server *server));
void launch(struct Server *server);

#endif