#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

int storage_init(const char* storage_dir) {
    // Attempt to create the directory if it doesn't exist
    return mkdir(storage_dir, 0777); 
}

int storage_save_chunk(const char* storage_dir, const char* hash, const uint8_t* data, size_t size) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.chunk", storage_dir, hash);

    // Check if it already exists
    FILE* check = fopen(filepath, "rb");
    if (check) {
        fclose(check);
        return 0; 
    }

    FILE* file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not create chunk file %s\n", filepath);
        return -1;
    }

    fwrite(data, 1, size, file);
    fclose(file);
    return 0;
}

void storage_sync_index(const char* storage_dir, BPlusTree* index) {
    DIR* dir = opendir(storage_dir);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        char* filename = entry->d_name;
        char* ext = strrchr(filename, '.');
        if (ext && strcmp(ext, ".chunk") == 0) {
            char hash[65];
            size_t hash_len = ext - filename;
            if (hash_len < 65) {
                strncpy(hash, filename, hash_len);
                hash[hash_len] = '\0';
                btree_insert(index, hash, 1);
            }
        }
    }
    closedir(dir);
}

uint64_t storage_get_size(const char* storage_dir) {
    uint64_t total_size = 0;
    
    DIR* dir = opendir(storage_dir);
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        char* filename = entry->d_name;
        char* ext = strrchr(filename, '.');
        if (ext && strcmp(ext, ".chunk") == 0) {
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", storage_dir, filename);
            struct stat stat_buf;
            if (stat(filepath, &stat_buf) == 0) {
                total_size += stat_buf.st_size;
            }
        }
    }
    closedir(dir);
    
    return total_size;
}

uint8_t* storage_get_chunk(const char* storage_dir, const char* hash, size_t* out_size) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s.chunk", storage_dir, hash);
    *out_size = 0;

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open chunk file %s\n", filepath);
        return NULL;
    }

    // Seek to the end to determine file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        fclose(file);
        return NULL;
    }

    uint8_t* data = (uint8_t*)malloc(size);
    if (data) {
        if (fread(data, 1, size, file) != (size_t)size) {
            free(data);
            data = NULL;
        } else {
            *out_size = size;
        }
    }
    
    fclose(file);
    return data;
}
