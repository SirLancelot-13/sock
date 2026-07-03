#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../functions/string_operations.c"

void response(int new_socket, char *buffer) {
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    if (getpeername(new_socket, (struct sockaddr *)&peer_addr, &peer_addr_len) == 0)
    {
        char peer_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), peer_ip, INET_ADDRSTRLEN);
        printf("IP Address of the peer: %s\n\n", peer_ip);
    }

    char *endpoint = extract_endpoint_type_shi(buffer);
    if (endpoint != NULL && strcmp(endpoint, "/") == 0) {
        char *response_str = "HTTP/1.1 200 OK\r\n"
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
        write(new_socket, response_str, strlen(response_str));
    }
    if (endpoint != NULL) {
        free(endpoint);
    }
    close(new_socket);
}
