#include "btree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static BPlusNode* create_node(bool is_leaf) {
    BPlusNode* node = (BPlusNode*)calloc(1, sizeof(BPlusNode));
    node->is_leaf = is_leaf;
    return node;
}

void btree_init(BPlusTree* tree) {
    if (tree) tree->root = NULL;
}

static void free_node(BPlusNode* node) {
    if (!node) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_node(node->children[i]);
        }
    }
    free(node);
}

void btree_free(BPlusTree* tree) {
    if (tree) {
        free_node(tree->root);
        tree->root = NULL;
    }
}

uint64_t btree_search(BPlusTree* tree, const char* key) {
    if (!tree || !tree->root) return (uint64_t)-1;

    BPlusNode* cursor = tree->root;
    while (!cursor->is_leaf) {
        int idx = 0;
        // Search exactly as we did with std::upper_bound
        while (idx < cursor->num_keys && strcmp(key, cursor->keys[idx]) >= 0) {
            idx++;
        }
        cursor = cursor->children[idx];
    }
    
    // Scan leaf node for match
    for (int i = 0; i < cursor->num_keys; i++) {
        if (strcmp(cursor->keys[i], key) == 0) {
            return cursor->values[i];
        }
    }

    return (uint64_t)-1;
}

static BPlusNode* find_parent(BPlusNode* cursor, BPlusNode* child) {
    if (cursor->is_leaf || cursor->children[0]->is_leaf) return NULL;

    for (int i = 0; i <= cursor->num_keys; i++) {
        if (cursor->children[i] == child) return cursor;
        BPlusNode* res = find_parent(cursor->children[i], child);
        if (res) return res;
    }
    return NULL;
}

static void insert_internal(const char* key, BPlusNode* parent, BPlusNode* child, BPlusTree* tree) {
    int idx = 0;
    while (idx < parent->num_keys && strcmp(parent->keys[idx], key) <= 0) idx++;

    // Shift keys and children natively
    for (int i = parent->num_keys; i > idx; i--) {
        strcpy(parent->keys[i], parent->keys[i - 1]);
    }
    for (int i = parent->num_keys + 1; i > idx + 1; i--) {
        parent->children[i] = parent->children[i - 1];
    }

    strcpy(parent->keys[idx], key);
    parent->children[idx + 1] = child;
    parent->num_keys++;

    if (parent->num_keys >= BTREE_M) {
        BPlusNode* new_internal = create_node(false);
        int mid = BTREE_M / 2;

        char up_key[65];
        strcpy(up_key, parent->keys[mid]);

        int j = 0;
        for (int i = mid + 1; i < parent->num_keys; i++) {
            strcpy(new_internal->keys[j], parent->keys[i]);
            new_internal->children[j] = parent->children[i];
            j++;
        }
        new_internal->children[j] = parent->children[parent->num_keys];
        new_internal->num_keys = parent->num_keys - mid - 1;
        parent->num_keys = mid;

        if (parent == tree->root) {
            BPlusNode* new_root = create_node(false);
            strcpy(new_root->keys[0], up_key);
            new_root->children[0] = parent;
            new_root->children[1] = new_internal;
            new_root->num_keys = 1;
            tree->root = new_root;
        } else {
            insert_internal(up_key, find_parent(tree->root, parent), new_internal, tree);
        }
    }
}

void btree_insert(BPlusTree* tree, const char* key, uint64_t value) {
    if (!tree) return;
    if (!tree->root) {
        tree->root = create_node(true);
        strcpy(tree->root->keys[0], key);
        tree->root->values[0] = value;
        tree->root->num_keys = 1;
        return;
    }

    BPlusNode* cursor = tree->root;
    BPlusNode* parent = NULL;

    while (!cursor->is_leaf) {
        parent = cursor;
        int idx = 0;
        while (idx < cursor->num_keys && strcmp(key, cursor->keys[idx]) >= 0) idx++;
        cursor = cursor->children[idx];
    }
    
    int idx = 0;
    while (idx < cursor->num_keys && strcmp(cursor->keys[idx], key) < 0) idx++;

    for (int i = cursor->num_keys; i > idx; i--) {
        strcpy(cursor->keys[i], cursor->keys[i - 1]);
        cursor->values[i] = cursor->values[i - 1];
    }
    
    strcpy(cursor->keys[idx], key);
    cursor->values[idx] = value;
    cursor->num_keys++;

    if (cursor->num_keys >= BTREE_M) {
        BPlusNode* new_leaf = create_node(true);
        int mid = BTREE_M / 2;
        
        int j = 0;
        for (int i = mid; i < cursor->num_keys; i++) {
            strcpy(new_leaf->keys[j], cursor->keys[i]);
            new_leaf->values[j] = cursor->values[i];
            j++;
        }
        
        new_leaf->num_keys = cursor->num_keys - mid;
        cursor->num_keys = mid;

        new_leaf->next = cursor->next;
        cursor->next = new_leaf;

        if (cursor == tree->root) {
            BPlusNode* new_root = create_node(false);
            strcpy(new_root->keys[0], new_leaf->keys[0]);
            new_root->children[0] = cursor;
            new_root->children[1] = new_leaf;
            new_root->num_keys = 1;
            tree->root = new_root;
        } else {
            insert_internal(new_leaf->keys[0], parent, new_leaf, tree);
        }
    }
}

// C array queue for BFS visualization
void btree_display(BPlusTree* tree) {
    if (!tree || !tree->root) {
        printf("Tree is empty.\n");
        return;
    }

    BPlusNode* q[1000];
    int head = 0, tail = 0;
    q[tail++] = tree->root;
    
    int level = 0;
    while (head < tail) {
        int sz = tail - head;
        printf("Level %d: ", level);
        for (int i = 0; i < sz; i++) {
            BPlusNode* cur = q[head++];
            printf("[");
            for (int j = 0; j < cur->num_keys; j++) {
                char short_key[7];
                strncpy(short_key, cur->keys[j], 6);
                short_key[6] = '\0';
                printf("%s", short_key);
                if (j < cur->num_keys - 1) printf("|");
            }
            printf("] ");
            if (!cur->is_leaf) {
                for (int j = 0; j <= cur->num_keys; j++) {
                    if (tail < 1000) q[tail++] = cur->children[j];
                }
            }
        }
        printf("\n");
        level++;
    }
}
