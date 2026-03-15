#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_DIGEST_SIZE 32
#define SHA224_256_BLOCK_SIZE 64

// Struct representing the SHA-256 Context instead of a C++ Class
typedef struct {
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    uint32_t m_h[8];
} SHA256_CTX;

// C function prototypes
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const unsigned char *message, unsigned int len);
void sha256_final(SHA256_CTX *ctx, unsigned char *digest);

// Helper function to process raw data and output a hex string
void sha256_hash_string(const unsigned char *data, size_t len, char outputBuffer[65]);

#endif
