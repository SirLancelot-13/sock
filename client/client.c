#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include "client.h"
#include "../server/server.h"
#include "../functions/string_operations.h"

int connect_to_server(const char *client_ip, const char *server_ip, int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = 0; // Bind to any random source port
    if (inet_pton(AF_INET, client_ip, &client_addr.sin_addr) <= 0)
    {
        perror("inet_pton client_ip");
        close(sock_fd);
        return 1;
    }

    if (bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("bind");
        close(sock_fd);
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton server");
        close(sock_fd);
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    char *request = send_get_request((char *)server_ip, "/");
    if (send(sock_fd, request, strlen(request), 0) < 0)
    {
        perror("send");
        free(request);
        close(sock_fd);
        return 1;
    }
    free(request);

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

int main(int argc, char **argv)
{
    if (argc != 2){
        printf("Insufficient Arguments.\n");
        return 1;
    }
    else{
        return connect_to_server(argv[1], "127.0.0.1", SERVER_PORT);
    }
}
