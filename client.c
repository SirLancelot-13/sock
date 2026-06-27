#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "client.h"
#include "server.h"

int connect_to_server(const char *ip, int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    const char *request = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    if (send(sock_fd, request, strlen(request), 0) < 0)
    {
        perror("send");
        close(sock_fd);
        return 1;
    }

    char response[4096];
    ssize_t received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (received < 0)
    {
        perror("recv");
        close(sock_fd);
        return 1;
    }

    response[received] = '\0';
    printf("%s", response);

    close(sock_fd);
    return 0;
}

int main(void)
{
    return connect_to_server("127.0.0.1", SERVER_PORT);
}

