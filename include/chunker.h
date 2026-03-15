#ifndef CHUNKER_H
#define CHUNKER_H

#include <stddef.h>
#include <stdint.h>

#define CHUNK_SIZE 4096

// C Structure holding chunk details
typedef struct {
    char hash[65];                 // 64-char hash + null terminator
    uint8_t* data;                 // Dynamically allocated binary data
    size_t size;                   // Actual size of this chunk (up to 4096)
    uint64_t offset;               // Position in original file
} ChunkInfo;

// Reads a file from disk, chunks it, hashes it, and returns a dynamic array of chunks.
// The num_chunks pointer will be updated with the final count.
// The caller is responsible for freeing the array by calling free_chunks.
ChunkInfo* chunk_file(const char* filepath, int* num_chunks);

// Frees the memory allocated by chunk_file
void free_chunks(ChunkInfo* chunks, int num_chunks);

#endif
