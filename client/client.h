#ifndef CLIENT_H
#define CLIENT_H

#include <stddef.h>

// socket client struct to be used via Cgo
typedef struct {
    int socket_fd;
    char server_ip[64];
    int port;
    int connected;
} sock_client_t;

// The cgo api functions
sock_client_t* sock_client_create(const char *server_ip, int port);
int sock_client_connect(sock_client_t *client, const char *username);
int sock_client_send_message(sock_client_t *client, const char *message);
char* sock_client_recv_message(sock_client_t *client, int timeout_ms);
void sock_client_free_string(char *str);
void sock_client_destroy(sock_client_t *client);

typedef struct {
    int sock_fd;
    char *ip;
    char *port;
    char *message;
} client_t;

int establish_connection(const char *client_ip, const char *server_ip, int port);
int send_message_to_server(const char *client_ip, const char *server_ip, int port, const char *message);
void send_websocket_client_text_frame(int fd, const char *message);
int connect_to_server(const char *client_ip, const char *server_ip, int port);
int run_realtime_chat(const char *server_ip, int port);

#endif
