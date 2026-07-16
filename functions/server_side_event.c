#include "server_side_event.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void broadcast_event(char *message, char *username, int client_sockets[], int num_clients){
    printf("Event broadcasted: %s - %s\n", username, message);
    for(int i = 0; i < num_clients; i++){
        write(client_sockets[i], message, strlen(message));
    }
}
