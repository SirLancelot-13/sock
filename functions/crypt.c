#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

void sha1_transform(uint32_t state[5], const uint8_t buffer[64]){
    uint32_t block[80];
    for (int i=0; i < 16; i++){
        block[i]=(buffer[i*4] << 24) | (buffer[i*4+1] << 16) | (buffer[i*4+2] << 8) | (buffer[i*4+3]);
    }
    for (int i=16; i<80; i++){
        block[i]=SHA1_ROL(block[i-3]^block[i-8]^block[i-14]^block[i-16], 1);
    }
    uint32_t a = state[0], b=state[1], c=state[2], d=state[3], e=state[4];
    for (int i=0; i<80; i++){
        uint32_t f,k;
        switch(i/20){
            case 0: f=(b & c) | ((-b) & d);
            k = 0x5A827999;
            break;
            case 1: f=b^c^d;
            k = 0x6ED9EBA1;
            break;
            case 2: f=(b&c) | (b&d) | (c&d);
            k = 0x8F1BBCDC;
            break;
            default: f=b^c^d;
            k = 0xCA62C1D6;
        }
        uint32_t temp = SHA1_ROL(a, 5)+f+e+k+block[i];
        e=d;d=c;c=SHA1_ROL(b, 30);b=a;a=temp;
    }
    state[0] += a; state[1]+=b; state[2]+=c; state[3]+=d; state[4]+=e;
}

void sha1_hash(const int8_t *data, size_t len, uint8_t digest[20]){
    uint32_t state[5]={0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    uint8_t buffer[64];
    size_t i=0;
    uint8_t total_size=len*8;
    while (len - i>64){
        memcpy(buffer, data+i, 64);
        sha1_transform(state, buffer);
        i+=64;
    }
    memset(buffer, 0, 64);
    size_t len_remaining=len-i;
    memcpy(buffer, data+i, len_remaining);
    buffer[len_remaining]=0x80;
    if (len_remaining >= 56){
        sha1_transform(state, buffer);
        memset(buffer, 0, 64);
    }
    for (int i=0; i<=7; i++){
        buffer[i]=(total_size >> (56-8*i)) & 0xFF;
    }
    sha1_transform(state, buffer);
    for (int j=0; j<5; j++){
        digest[j * 4] = (state[j] >> 24) & 0xFF;
        digest[j * 4 + 1] = (state[j] >> 16) & 0xFF;
        digest[j * 4 + 2] = (state[j] >> 8) & 0xFF;
        digest[j * 4 + 3] = state[j] & 0xFF;
    }
}
//Table for base-64 conversion.
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void base64_encode(const uint8_t *src, size_t len, char *out) {
    size_t i = 0, j = 0;
    for (; i < len - (len % 3); i += 3) {
        out[j++] = base64_table[src[i] >> 2];
        out[j++] = base64_table[((src[i] & 0x03) << 4) | (src[i + 1] >> 4)];
        out[j++] = base64_table[((src[i + 1] & 0x0F) << 2) | (src[i + 2] >> 6)];
        out[j++] = base64_table[src[i + 2] & 0x3F];
    }
    if (len % 3 == 1) {
        out[j++] = base64_table[src[i] >> 2];
        out[j++] = base64_table[(src[i] & 0x03) << 4];
        out[j++] = '=';
        out[j++] = '=';
    } else if (len % 3 == 2) {
        out[j++] = base64_table[src[i] >> 2];
        out[j++] = base64_table[((src[i] & 0x03) << 4) | (src[i + 1] >> 4)];
        out[j++] = base64_table[(src[i + 1] & 0x0F) << 2];
        out[j++] = '=';
    }
    out[j] = '\0';
}
