#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * BitVector tracks allocated blocks in the storage file.
 * Each bit represents one 4KB block. 1 = Used, 0 = Free.
 */
typedef struct {
    uint8_t* bits;
    size_t num_bytes;
} BitVector;

/**
 * Initializes a new BitVector.
 * @param bv Pointer to the BitVector struct.
 * @param numBits Initial number of bits (blocks) to track.
 */
void bitvector_init(BitVector* bv, size_t numBits);

/**
 * Frees the dynamically allocated memory of the BitVector.
 */
void bitvector_free(BitVector* bv);

/**
 * Marks a block as used. Expands dynamically if necessary.
 */
void bitvector_set(BitVector* bv, size_t index);

/**
 * Checks if a block is used.
 */
bool bitvector_test(const BitVector* bv, size_t index);

/**
 * Resets a block to free.
 */
void bitvector_reset(BitVector* bv, size_t index);

#endif
