#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "crypt.h"
int read_websocket_frame(int fd, char *payload_o, size_t maxlen, int *optcode_out){
    uint8_t header[2];
    if (recv(fd, header, 2, 0) <= 0){
        perror("Failed to extract header.\n");
        return -1;
    }
    uint8_t fin=(header[0]>>7)&0x01;
    (void)fin;
    if (optcode_out) {
        *optcode_out = header[0] & 0x0F;
    }
    uint8_t masked=(header[1]>>7)&0x01;
    //Extracting the length from the websocket request.
    uint8_t payload_len=(header[1])&0x7F;
    if (payload_len == 126){
        uint8_t full_len[2];
        if (recv(fd, full_len, 2, 0)<=0){
            perror("Failed to get full length for the packet.\n");
            return -1;
        }
        payload_len=(full_len[0]<<8)|(full_len[1]); //2 extra bytes in the websocket reserved for extra length.
    } else if (payload_len == 126){
        uint8_t full_len[8];
        if (recv(fd, full_len, 8, 0) <= 0){
            perror("Failed to get full length of the packet.\n");
            return -1;
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
            return -1;
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

void send_ws_frame(int fd, const char *http_req){
    size_t len=strlen(http_req);
    uint8_t header[10];
    header[0]=0x81; //FIN bit set, text-frame
    int header_len=2;
    if (len<126){
        header[1]=len;
    } else if (len<65536) {
        header[1]=126;
        header[2]=(len>>8)&0xFF;
        header[3]=len&0xFF;
        header_len+=2;
    } else{
        header[1]=127;
        header_len+=8;
        for (int i=0; i<8; i++){
            header[2+i]=(len>>(56-8*i))&0xFF;
        }
    }
    send(fd, header, header_len, 0);
    send(fd, http_req, len, 0);
}

int handle_ws_frame(int fd, const char *http_req){
    printf("%s",http_req);
    const char *key_hdr="Sec-WebSocket-Key: ";
    char *key_start=strstr(http_req, key_hdr);
    if (!key_start){
        perror("Failed to parse in the start of the http-request.\n");
        return -1;
    }
    char *key_end=strstr(http_req, "\r\n");
    if (!key_end){
        perror("Failed to parse in the end of the http-request.\n");
        return -1;
    }
    size_t key_len=key_end-key_start-strlen(key_hdr);
    char key[128];
    strncpy(key, key_start+strlen(key_hdr), key_len);
    key[key_len]='\0';
    // Concatenate key with GUID.
    char concatenate[256];
    snprintf(concatenate, sizeof(concatenate), "%s%s", key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    // Hash ts
    uint8_t hash[20];
    sha1_hash((const uint8_t *)concatenate, strlen(concatenate), hash);
    // Convert to b64
    char b64[32];
    base64_encode(hash, 20, b64);
    // Send response
    char response[512];
    snprintf(response, sizeof(response),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n"
             "\r\n", b64);
    send(fd, response, strlen(response), 0);
    return 0;
}
