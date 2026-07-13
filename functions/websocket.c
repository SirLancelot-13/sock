#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

int read_websocket_frame(int fd, char *payload_o, size_t maxlen, int *optcode_out){
    uint8_t header[2];
    if (recv(fd, header, 2, 0) <= 0){
        perror("Failed to extract header.\n");
    }
    uint8_t fin=(header[0]>>7)&0x01;
    *optcode_out=header[0]&0x0F;
    uint8_t masked=(header[1]>>7)&0x01;
    //Extracting the length from the websocket request.
    uint8_t payload_len=(header[1])&0x7F;
    if (payload_len == 126){
        uint8_t full_len[2];
        if (recv(fd, full_len, 2, 0)<=0){
            perror("Failed to get full length for the packet.\n");
        }
        payload_len=(full_len[0]<<8)|(full_len[1]); //2 extra bytes in the websocket reserved for extra length.
    } else if (payload_len == 126){
        uint8_t full_len[8];
        if (recv(fd, full_len, 8, 0) <= 0){
            perror("Failed to get full length of the packet.\n");
        }
        uint8_t payload_len=0;
        for (int i=0; i<8; i++){
            payload_len=payload_len<<8|full_len[i];
        }
    }
    //Masking and shi for websocket request.
    uint8_t masking_key[4]={0};
    if (masked) {
        if (recv(fd, masking_key, 4, 0)<=0){
            perror("Failed to get the masking key.\n");
        }
    }
    if (payload_len>=maxlen) payload_len=maxlen-1;
    size_t bytes_recieved=0;
    while (bytes_recieved<payload_len){
        size_t r=recv(fd, payload_o+bytes_recieved, payload_len-bytes_recieved, 0);
        bytes_recieved+=r;
    }
    payload_o[payload_len]='\0';

    //If data is masked.
    if (masked){
        for (size_t i=0; i<payload_len; i++){
            payload_o[i]=payload_o[i]^masking_key[i%4];
        }
    }

    return (int)payload_len;
}

void send_ws_text_frame({

})
