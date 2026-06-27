#include "server.h"

int main() {
    struct Server server = server_constructor(AF_INET, SERVER_PORT, SOCK_STREAM, 0, 10, INADDR_ANY, launch);
    server.launch(&server);
    return 0;
}