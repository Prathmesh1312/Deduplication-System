#include "bitvector.h"
#include <stdlib.h>
#include <string.h>

void bitvector_init(BitVector* bv, size_t numBits) {
    if (!bv) return;
    
    // Default to at least 1024 bits if 0 is passed
    if (numBits == 0) numBits = 1024;
    
    bv->num_bytes = (numBits + 7) / 8;
    bv->bits = (uint8_t*)calloc(bv->num_bytes, sizeof(uint8_t));
}

void bitvector_free(BitVector* bv) {
    if (bv && bv->bits) {
        free(bv->bits);
        bv->bits = NULL;
        bv->num_bytes = 0;
    }
}

void bitvector_set(BitVector* bv, size_t index) {
    if (!bv) return;
    
    size_t byte_idx = index / 8;
    
    if (byte_idx >= bv->num_bytes) {
        size_t new_bytes = byte_idx + 1;
        uint8_t* new_bits = (uint8_t*)realloc(bv->bits, new_bytes);
        if (!new_bits) return; // Reallocation failed
        
        // Zero out newly allocated memory
        memset(new_bits + bv->num_bytes, 0, new_bytes - bv->num_bytes);
        
        bv->bits = new_bits;
        bv->num_bytes = new_bytes;
    }
    
    bv->bits[byte_idx] |= (1 << (index % 8));
}

bool bitvector_test(const BitVector* bv, size_t index) {
    if (!bv || !bv->bits) return false;
    
    size_t byte_idx = index / 8;
    if (byte_idx >= bv->num_bytes) return false;
    
    return (bv->bits[byte_idx] & (1 << (index % 8))) != 0;
}

void bitvector_reset(BitVector* bv, size_t index) {
    if (!bv || !bv->bits) return;
    
    size_t byte_idx = index / 8;
    if (byte_idx >= bv->num_bytes) return;
    
    bv->bits[byte_idx] &= ~(1 << (index % 8));
}
