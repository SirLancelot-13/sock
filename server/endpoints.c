#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../functions/string_operations.c"
#include "../functions/db_operations.h"

void response(int new_socket, char *buffer) {
    char peer_ip[INET_ADDRSTRLEN] = {0};
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    if (getpeername(new_socket, (struct sockaddr *)&peer_addr, &peer_addr_len) == 0)
    {
        inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), peer_ip, INET_ADDRSTRLEN);
        printf("IP Address of the peer: %s\n\n", peer_ip);
    }

    char *endpoint = extract_endpoint_type_shi(buffer);
    if (endpoint != NULL && strcmp(endpoint, "/") == 0) {
        enum Request req_type = get_request_type(buffer);
        if (req_type == get) {
            char *username = NULL;
            if (peer_ip[0] != '\0') {
                username = get_username(db, peer_ip);
            }

            const char *display_name = (username != NULL) ? username : "no username";
            char *messages_str = get_last_10_messages(db);

            char response_body[4096];
            if (messages_str) {
                snprintf(response_body, sizeof(response_body), "%s\n%s", display_name, messages_str);
                free(messages_str);
            } else {
                snprintf(response_body, sizeof(response_body), "%s\n", display_name);
            }

            char response_str[8192];
            snprintf(response_str, sizeof(response_str),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n"
                     "%s",
                     strlen(response_body), response_body);

            write(new_socket, response_str, strlen(response_str));

            if (username) {
                free(username);
            }
        } else if (req_type == post) {
            char *username = NULL;
            if (peer_ip[0] != '\0') {
                username = get_username(db, peer_ip);
            }
            const char *sender = (username != NULL) ? username : "anonymous";

            char *message = extract_post_content(buffer);
            if (message != NULL) {
                insert_message(db, sender, message);
                free(message);
            }

            const char *response_body = "Message received";
            char response_str[512];
            snprintf(response_str, sizeof(response_str),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n"
                     "%s",
                     strlen(response_body), response_body);
            write(new_socket, response_str, strlen(response_str));

            if (username) {
                free(username);
            }
        }
    }
    else if (endpoint != NULL && strcmp(endpoint, "/login") == 0) {
        enum Request req_type2 = get_request_type(buffer);
        if (req_type2 == post) {
            char *username = extract_post_content(buffer);
            if (username != NULL) {
                if (peer_ip[0] != '\0') {
                    insert_or_ignore(db, peer_ip, username);
                }
                free(username);
            }

            const char *response_body = "Login registered";
            char response_str[512];
            snprintf(response_str, sizeof(response_str),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n"
                     "%s",
                     strlen(response_body), response_body);
            write(new_socket, response_str, strlen(response_str));
        }
    }
    if (endpoint != NULL) {
        free(endpoint);
    }
    close(new_socket);
}
