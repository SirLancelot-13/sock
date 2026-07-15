#ifndef CRYPT_H
#define CRYPT_H
#include <stdint.h>
#include <stddef.h>
void sha1_transform(uint32_t state[5], const uint8_t buffer[64]);
void sha1_hash(const int8_t *data, size_t len, uint8_t digest[20]);
void base64_encode(const uint8_t *src, size_t len, char *out);
#endif // CRYPT_H
