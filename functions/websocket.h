#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stddef.h>

int read_websocket_frame(int fd, char *payload_out, size_t max_len, int *opcode_out);
void send_ws_frame(int fd, const char *message);
int handle_ws_frame(int fd, const char *http_req);

#endif
