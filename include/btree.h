#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include <stdbool.h>

#define BTREE_M 4

// Standard C BPlusNode structure
typedef struct BPlusNode {
    bool is_leaf;
    int num_keys;
    char keys[BTREE_M][65];
    uint64_t values[BTREE_M]; // for leaf
    struct BPlusNode* children[BTREE_M + 1]; // for internal
    struct BPlusNode* next; // for leaf link list
} BPlusNode;

// Struct to track the tree root
typedef struct {
    BPlusNode* root;
} BPlusTree;

// C procedural operations for the B+ Tree
void btree_init(BPlusTree* tree);
void btree_free(BPlusTree* tree);

void btree_insert(BPlusTree* tree, const char* key, uint64_t value);
uint64_t btree_search(BPlusTree* tree, const char* key);

void btree_display(BPlusTree* tree);

#endif
