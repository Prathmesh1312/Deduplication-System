#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "chunker.h"
#include "storage.h"
#include "btree.h"

void print_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  Store/Deduplicate: ./dedup store <filepath> [--show-tree]\n");
    fprintf(stderr, "  Restore/Rebuild:   ./dedup restore <recipe_path> <output_path>\n");
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    const char* command = argv[1];

    printf("--- Smart Storage Deduplication System ---\n");

    const char* storage_dir = "unique_chunks";
    storage_init(storage_dir);

    BPlusTree index;
    btree_init(&index);

    if (strcmp(command, "store") == 0) {
        const char* filepath = argv[2];
        bool show_tree = false;

        if (strcmp(filepath, "--show-tree") == 0 && argc >= 4) {
            show_tree = true;
            filepath = argv[3];
        } else if (argc >= 4 && strcmp(argv[3], "--show-tree") == 0) {
            show_tree = true;
        }

        printf("Processing file: %s\n\n", filepath);

        storage_sync_index(storage_dir, &index);

        int num_chunks = 0;
        ChunkInfo* chunks = chunk_file(filepath, &num_chunks);

        if (!chunks || num_chunks == 0) {
            fprintf(stderr, "No chunks generated. Ensure the file exists and is readable.\n");
            if (chunks) free_chunks(chunks, num_chunks);
            btree_free(&index);
            return 1;
        }

        char recipe_path[1024];
        snprintf(recipe_path, sizeof(recipe_path), "%s.recipe", filepath);
        FILE* recipe_file = fopen(recipe_path, "w");
        if (!recipe_file) {
            fprintf(stderr, "Failed to create recipe file!\n");
            free_chunks(chunks, num_chunks);
            btree_free(&index);
            return 1;
        }

        int unique_chunks_count = 0;
        int duplicate_chunks = 0;
        uint64_t total_saved_space = 0;

        int i;
        for (i = 0; i < num_chunks; ++i) {
            char* hash = chunks[i].hash;
            fprintf(recipe_file, "%s\n", hash);

            uint64_t existing_offset = btree_search(&index, hash);

            if (existing_offset == (uint64_t)-1) {
                if (storage_save_chunk(storage_dir, hash, chunks[i].data, chunks[i].size) == 0) {
                    btree_insert(&index, hash, 1);
                    unique_chunks_count++;
                    char short_hash[9];
                    strncpy(short_hash, hash, 8);
                    short_hash[8] = '\0';
                    printf("[STORED] Chunk %d -> Hash: %s... saved in 'unique_chunks/'\n", i, short_hash);
                }
            } else {
                duplicate_chunks++;
                total_saved_space += chunks[i].size;
                char short_hash[9];
                strncpy(short_hash, hash, 8);
                short_hash[8] = '\0';
                printf("[SKIPPED] Chunk %d -> Hash: %s... already exists.\n", i, short_hash);
            }
        }
        
        fclose(recipe_file);

        printf("\n--- Summary ---\n");
        printf("Total Chunks Processed: %d\n", num_chunks);
        printf("New Unique Chunks:      %d\n", unique_chunks_count);
        printf("Duplicate Chunks:       %d\n", duplicate_chunks);
        printf("Total Saved Space:      %llu bytes\n", (unsigned long long)total_saved_space);
        printf("Storage Directory Size: %llu bytes\n", (unsigned long long)storage_get_size(storage_dir));
        printf("\nFile recipe saved to: %s\n", recipe_path);
        printf("Keep this recipe to restore your file later!\n");

        if (show_tree) {
            printf("\n--- B+ Tree Index Structure ---\n");
            btree_display(&index);
            printf("-------------------------------\n");
        }

        free_chunks(chunks, num_chunks);

    } else if (strcmp(command, "restore") == 0) {
        if (argc < 4) {
             fprintf(stderr, "Error: Missing output path for restore.\n");
             print_usage();
             btree_free(&index);
             return 1;
        }
        const char* recipe_path = argv[2];
        const char* output_path = argv[3];

        printf("Restoring file from recipe: %s\n", recipe_path);

        FILE* recipe_file = fopen(recipe_path, "r");
        if (!recipe_file) {
            fprintf(stderr, "Error: Could not open recipe file %s\n", recipe_path);
            btree_free(&index);
            return 1;
        }

        FILE* output_file = fopen(output_path, "wb");
        if (!output_file) {
            fprintf(stderr, "Error: Could not create output file %s\n", output_path);
            fclose(recipe_file);
            btree_free(&index);
            return 1;
        }

        char hash[128];
        int chunks_restored = 0;
        
        while (fgets(hash, sizeof(hash), recipe_file)) {
            // Trim newline characters safely
            size_t len = strlen(hash);
            while (len > 0 && (hash[len - 1] == '\n' || hash[len - 1] == '\r')) {
                hash[len - 1] = '\0';
                len--;
            }

            if (len == 0) continue;

            size_t data_size = 0;
            uint8_t* data = storage_get_chunk(storage_dir, hash, &data_size);
            
            if (!data) {
                fprintf(stderr, "CRITICAL ERROR: Missing chunk %s in unique_chunks directory!\n", hash);
                fprintf(stderr, "Cannot reconstruct file. Restoration aborted.\n");
                fclose(output_file);
                fclose(recipe_file);
                btree_free(&index);
                return 1;
            }

            fwrite(data, 1, data_size, output_file);
            free(data); // Important memory management block loop
            chunks_restored++;
        }

        fclose(recipe_file);
        fclose(output_file);

        printf("\n--- Restoration Complete ---\n");
        printf("Chunks restored: %d\n", chunks_restored);
        printf("File successfully rebuilt to: %s\n", output_path);

    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage();
        btree_free(&index);
        return 1;
    }

    btree_free(&index);
    return 0;
}
