#ifndef client_h
#define client_h
int establish_connection(const char *client_ip, const char *server_ip, int port);
int send_message_to_server(const char *client_ip, const char *server_ip, int port, const char *message);
void send_websocket_client_text_frame(int fd, const char *message);
int connect_to_server(const char *client_ip, const char *server_ip, int port);
int run_realtime_chat(const char *server_ip, int port);
int main_interface(int argc, char **argv);

#endif
