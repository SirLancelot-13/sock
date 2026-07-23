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

int establish_connection(const char *client_ip, const char *server_ip, int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return -1;
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
        return -1;
    }

    if (bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("bind");
        close(sock_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton server");
        close(sock_fd);
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

int connect_to_server(const char *client_ip, const char *server_ip, int port)
{
    int sock_fd = establish_connection(client_ip, server_ip, port);
    if (sock_fd < 0)
    {
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

int send_message_to_server(const char *client_ip, const char *server_ip, int port, const char *message)
{
    int sock_fd = establish_connection(client_ip, server_ip, port);
    if (sock_fd < 0)
    {
        return 1;
    }

    char *request = send_post_request((char *)server_ip, "/", (char *)message);
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
    printf("%s\n", response);

    close(sock_fd);
    return 0;
}

#include "../functions/websocket.h"
#include <sys/select.h>

void send_websocket_client_text_frame(int fd, const char *message) {
    size_t len = strlen(message);
    uint8_t header[14];
    header[0] = 0x81; // FIN = 1, Opcode = 1 (Text)

    int header_len = 2;
    if (len < 126) {
        header[1] = len | 0x80; // MASK = 1, payload len
    } else if (len <= 65535) {
        header[1] = 126 | 0x80;
        header[2] = (len >> 8) & 0xFF;
        header[3] = len & 0xFF;
        header_len = 4;
    } else {
        header[1] = 127 | 0x80;
        for (int i = 0; i < 8; i++) {
            header[2 + i] = (len >> ((7 - i) * 8)) & 0xFF;
        }
        header_len = 10;
    }

    // Generate a mask key (simple client hardcoded/pseudo-random key)
    uint8_t masking_key[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    memcpy(header + header_len, masking_key, 4);
    header_len += 4;

    // XOR Encrypt Payload
    uint8_t *masked_payload = malloc(len);
    for (size_t i = 0; i < len; i++) {
        masked_payload[i] = message[i] ^ masking_key[i % 4];
    }

    send(fd, header, header_len, 0);
    send(fd, masked_payload, len, 0);
    free(masked_payload);
}

int run_realtime_chat(const char *server_ip, int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
        return -1;
    }

    // Send standard HTTP WebSocket upgrade request
    char handshake[512];
    snprintf(handshake, sizeof(handshake),
             "GET /ws HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
             "Sec-WebSocket-Version: 13\r\n\r\n",
             server_ip, port);
    send(sock_fd, handshake, strlen(handshake), 0);

    // Read the server handshake response
    char response[1024];
    ssize_t received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (received <= 0 || !strstr(response, "101 Switching Protocols")) {
        printf("WebSocket handshake failed.\n");
        close(sock_fd);
        return -1;
    }
    printf("Handshake succeeded. You are now in the real-time chat!\n");

    fd_set read_fds;
    char input_buffer[256];
    char recv_buffer[BUFFER_SIZE];

    printf("Enter message: ");
    fflush(stdout);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock_fd, &read_fds);

        int activity = select(sock_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // A. User entered a message on the console
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

            // Remove newline
            input_buffer[strcspn(input_buffer, "\n")] = '\0';
            if (strcmp(input_buffer, "exit") == 0) break;

            send_websocket_client_text_frame(sock_fd, input_buffer);
            printf("Enter message: ");
            fflush(stdout);
        }

        // B. Server sent a broadcasted message
        if (FD_ISSET(sock_fd, &read_fds)) {
            int opcode;
            int len = read_websocket_frame(sock_fd, recv_buffer, sizeof(recv_buffer) - 1, &opcode);
            if (len <= 0) {
                printf("\nDisconnected from server.\n");
                break;
            }
            recv_buffer[len] = '\0';
            printf("\n%s\n", recv_buffer);
            printf("Enter message: ");
            fflush(stdout);
        }
    }

    close(sock_fd);
    return 0;
}

int main(int argc, char **argv) //Creative renaming. Yes, I know.
{
    if (argc < 2){
        printf("Insufficient Arguments.\n");
        return 1;
    }

    const char *username = argv[1];
    const char *client_ip = (argc >= 3) ? argv[2] : "127.0.0.1";

    /* Register username with server via POST /login */
    {
        int sock_fd = establish_connection(client_ip, "127.0.0.1", SERVER_PORT);
        if (sock_fd >= 0) {
            char *req = send_post_request((char *)"127.0.0.1", "/login", (char *)username);
            send(sock_fd, req, strlen(req), 0);
            free(req);
            char resp[512];
            recv(sock_fd, resp, sizeof(resp) - 1, 0);
            close(sock_fd);
        } else {
            printf("Failed to register username with server.\n");
            return 1;
        }
    }

    int login_res = connect_to_server(client_ip, "127.0.0.1", SERVER_PORT);
    if (login_res != 0) {
        printf("Failed to connect/log in to server.\n");
        return login_res;
    }

    // Call interactive real-time WebSocket chat
    return run_realtime_chat("127.0.0.1", SERVER_PORT);
}

/* ========================================================================= */
/* Cgo Explicit API Implementation                                           */
/* ========================================================================= */

sock_client_t* sock_client_create(const char *server_ip, int port) {
    sock_client_t *client = (sock_client_t *)calloc(1, sizeof(sock_client_t));
    if (!client) return NULL;

    if (server_ip) {
        strncpy(client->server_ip, server_ip, sizeof(client->server_ip) - 1);
    } else {
        strncpy(client->server_ip, "127.0.0.1", sizeof(client->server_ip) - 1);
    }
    client->port = port > 0 ? port : SERVER_PORT;
    client->socket_fd = -1;
    client->connected = 0;

    return client;
}

int sock_client_connect(sock_client_t *client, const char *username) {
    if (!client) return -1;

    // Optional login registration via HTTP POST /login
    if (username && strlen(username) > 0) {
        int reg_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (reg_fd >= 0) {
            struct sockaddr_in srv_addr;
            memset(&srv_addr, 0, sizeof(srv_addr));
            srv_addr.sin_family = AF_INET;
            srv_addr.sin_port = htons(client->port);
            inet_pton(AF_INET, client->server_ip, &srv_addr.sin_addr);

            if (connect(reg_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == 0) {
                char *req = send_post_request(client->server_ip, "/login", (char *)username);
                if (req) {
                    send(reg_fd, req, strlen(req), 0);
                    free(req);
                    char resp[512];
                    recv(reg_fd, resp, sizeof(resp) - 1, 0);
                }
            }
            close(reg_fd);
        }
    }

    // Establish main WebSocket socket connection
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return -1;

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->port);

    if (inet_pton(AF_INET, client->server_ip, &server_addr.sin_addr) <= 0) {
        close(sock_fd);
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock_fd);
        return -1;
    }

    // Send HTTP WebSocket Upgrade request
    char handshake[512];
    snprintf(handshake, sizeof(handshake),
             "GET /ws HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
             "Sec-WebSocket-Version: 13\r\n\r\n",
             client->server_ip, client->port);

    if (send(sock_fd, handshake, strlen(handshake), 0) < 0) {
        close(sock_fd);
        return -1;
    }

    char response[1024];
    ssize_t received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (received <= 0 || !strstr(response, "101 Switching Protocols")) {
        close(sock_fd);
        return -1;
    }

    client->socket_fd = sock_fd;
    client->connected = 1;
    return 0;
}

int sock_client_send_message(sock_client_t *client, const char *message) {
    if (!client || !client->connected || client->socket_fd < 0 || !message) {
        return -1;
    }

    send_websocket_client_text_frame(client->socket_fd, message);
    return 0;
}

char* sock_client_recv_message(sock_client_t *client, int timeout_ms) {
    if (!client || !client->connected || client->socket_fd < 0) {
        return NULL;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(client->socket_fd, &read_fds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int activity = select(client->socket_fd + 1, &read_fds, NULL, NULL, &tv);
    if (activity > 0 && FD_ISSET(client->socket_fd, &read_fds)) {
        char recv_buffer[4096];
        int opcode = 0;
        int len = read_websocket_frame(client->socket_fd, recv_buffer, sizeof(recv_buffer) - 1, &opcode);
        if (len > 0) {
            recv_buffer[len] = '\0';
            return strdup(recv_buffer);
        } else if (len <= 0) {
            client->connected = 0;
        }
    }

    return NULL;
}

void sock_client_free_string(char *str) {
    if (str) {
        free(str);
    }
}

void sock_client_destroy(sock_client_t *client) {
    if (client) {
        if (client->socket_fd >= 0) {
            close(client->socket_fd);
            client->socket_fd = -1;
        }
        client->connected = 0;
        free(client);
    }
}
