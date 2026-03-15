#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include <stddef.h>
#include "chunker.h"
#include "btree.h"

// Initialize the storage directory
int storage_init(const char* storage_dir);

// Saves a raw binary chunk 
// Returns 0 on success, -1 on failure
int storage_save_chunk(const char* storage_dir, const char* hash, const uint8_t* data, size_t size);

// Scans the directory and inserts all known chunks into the B+ Tree
void storage_sync_index(const char* storage_dir, BPlusTree* index);

// Recursively calculates the disk space consumed by files in the directory
uint64_t storage_get_size(const char* storage_dir);

// Reads a chunk from disk into memory. 
// Uses an output pointer *out_size to record the array size.
// Returns a dynamically allocated array. The caller must free() it!
uint8_t* storage_get_chunk(const char* storage_dir, const char* hash, size_t* out_size);

#endif
