#ifndef client_h
#define client_h

int connect_to_server(const char *client_ip, const char *server_ip, int port);
int run_realtime_chat(const char *server_ip, int port);

#endif
