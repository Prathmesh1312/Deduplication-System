#include "chunker.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ChunkInfo* chunk_file(const char* filepath, int* num_chunks) {
    *num_chunks = 0;
    
    // Open in raw binary reading mode
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s\n", filepath);
        return NULL;
    }
    
    // Start with a dynamic array for 10 chunks, we will realloc if needed
    int capacity = 10;
    ChunkInfo* chunks = (ChunkInfo*)malloc(capacity * sizeof(ChunkInfo));
    if (!chunks) {
        fclose(file);
        return NULL;
    }
    
    uint64_t current_offset = 0;
    uint8_t buffer[CHUNK_SIZE];
    
    // Read until End Of File
    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, CHUNK_SIZE, file);
        
        if (bytes_read > 0) {
            // If our dynamic array is full, double its capacity
            if (*num_chunks >= capacity) {
                capacity *= 2;
                ChunkInfo* new_chunks = (ChunkInfo*)realloc(chunks, capacity * sizeof(ChunkInfo));
                if (!new_chunks) {
                    fprintf(stderr, "Memory reallocation failed!\n");
                    break;
                }
                chunks = new_chunks;
            }
            
            ChunkInfo* info = &chunks[*num_chunks];
            
            // Allocate heap memory precisely for the data read
            info->data = (uint8_t*)malloc(bytes_read);
            if (info->data) {
                memcpy(info->data, buffer, bytes_read);
            }
            
            info->size = bytes_read;
            info->offset = current_offset;
            
            // Compute the SHA256 of the raw data buffer we just read
            sha256_hash_string(info->data, info->size, info->hash);
            
            (*num_chunks)++;
            current_offset += bytes_read;
        }
    }
    
    fclose(file);
    return chunks;
}

void free_chunks(ChunkInfo* chunks, int num_chunks) {
    if (!chunks) return;
    
    // First, free all the internal data buffers
    int i;
    for (i = 0; i < num_chunks; i++) {
        if (chunks[i].data) {
            free(chunks[i].data);
        }
    }
    
    // Then free the array struct itself
    free(chunks);
}
