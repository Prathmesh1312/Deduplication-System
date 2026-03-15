# Smart Storage Deduplication: Architecture & Explanation

This document explains the inner workings of the Smart Storage Deduplication project, detailing the purpose of each file, the advanced data structures used, definitions of complex terms, and the step-by-step workflow of the application.

---

## 1. Glossary of Terminology

*   **Deduplication (Dedup):** A specialized data compression technique that eliminates duplicate copies of repeating data. Instead of saving the same file (or parts of a file) multiple times, the system saves just one copy and references it elsewhere.
*   **Chunk / Block:** A small, fixed-size piece of data. Instead of processing a 1GB file all at once, the system breaks it down into "chunks" (in this project, exactly 4096 bytes or 4KB). This allows the system to find duplicate parts *inside* different files.
*   **Hash / SHA-256:** A mathematical algorithm that acts like a digital fingerprint. If you feed the SHA-256 algorithm 4KB of data, it will output a unique 64-character string (e.g., `35351cef...`). If two chunks have the *exact same data*, they will produce the *exact same hash*.
*   **Recipe (`.recipe` file):** Since deduplication breaks a file into hundreds of chunks and only saves the unique ones, we need a way to build the file back! A "recipe" is simply a text file containing the list of hashes, in exact order, that make up the original file. Think of it like instructions: "To rebuild demo.txt, take Chunk A, then Chunk B, then Chunk A again."
*   **Index:** A fast-search directory (like a book's index). The system uses an index to quickly remember which chunk hashes it has already saved to the disk.

---

## 2. File Functions and Purposes

The project is organized into modular components. Here is what each file actually does:

### Source Files (`src/`) & Headers (`include/`)
*   **`main.cpp`**: The heart of the program. It reads your terminal commands (`store` or `restore`), orchestrates the components (calls the chunker, checks the index, talks to storage), prints the summary metrics to your screen, and manages the creation/reading of `.recipe` files.
*   **`chunker.cpp` & `chunker.h`**: The File Splitter. It opens your target file, reads it precisely 4KB at a time, and passes that raw binary data to the SHA-256 algorithm to generate hashes.
*   **`sha256.cpp` & `sha256.h`**: The Cryptographer. Contains the complex mathematics required to securely generate the 256-bit hashes. It is a standalone implementation of the industry-standard cryptographic algorithm.
*   **`storage.cpp` & `storage.h`**: The Disk Manager. This file actually interacts with your OS filesystem (macOS/Linux). It writes new raw binary chunks to the `unique_chunks/` folder and reads them back when requested. It also syncs the folder with the B+ Tree on startup.
*   **`btree.cpp` & `btree.h`**: The Search Engine. Contains the custom implementation of the B+ Tree data structure used for instantaneous hash lookups.
*   **`bitvector.h`**: A utility class representing an array of bits (1s and 0s). While chunks are currently saved as individual files, this bit vector could be used if you wanted to pack all the chunks into a single massive continuous block storage file (like `storage.dat`).

---

## 3. Advanced Data Structure Concepts

This project utilizes a **B+ Tree** (B-Plus Tree) as its core Advanced Data Structure.

### Why a B+ Tree?
If you have 1 million chunks in your `unique_chunks/` folder, checking your hard drive 1 million times to see if a file exists before saving a new chunk is incredibly slow (Disk I/O bottleneck). 
Instead, we use a B+ Tree stored in the computer's blazing-fast RAM. 

*   **Logarithmic Search `O(log n)`:** The B+ Tree is self-balancing. Searching for a hash amongst millions takes only a few operations natively. 
*   **High Fanout:** B+ trees have many children per node (Degree / M). This means the tree is very "shallow", requiring fewer steps to reach the bottom leaf nodes where the data resides.
*   **In-Memory Index:** In this project, when the program boots up, it scans the `unique_chunks/` folder once, puts every hash it finds into the B+ Tree, and then consults the B+ Tree for all deduplication decisions. 

---

## 4. Workflows

### The `store` Workflow (Deduplicating)
When you run `./dedup store test.txt`:
1.  **Boot & Sync:** The `StorageManager` looks inside `unique_chunks/`. It reads every file name and inserts the hashes into the `BPlusTree`.
2.  **Chunking:** The `FileChunker` reads `test.txt` and cuts it into a list of 4KB chunks, hashing each one.
3.  **Deduplication Loop:** For each chunk in the file:
    *   The system searches the `BPlusTree` for the hash.
    *   **If found (Duplicate):** Do nothing! Save space.
    *   **If NOT found (Unique):** Tell `StorageManager` to create a new `.chunk` file on the hard drive. Insert the new hash into the `BPlusTree`.
4.  **Recipe Generation:** Simultaneously, the system writes every hash it processes (unique or duplicate) in sequential chronological order into `test.txt.recipe`.

### The `restore` Workflow (Rebuilding)
When you run `./dedup restore test.txt.recipe restored.txt`:
1.  **Initialization:** The system opens the `.recipe` file in read mode and creates an empty `restored.txt` file in write mode.
2.  **Parsing:** The system reads the `.recipe` file line-by-line, retrieving one hash string at a time.
3.  **Data Retrieval:** It asks the `StorageManager` to find the `.chunk` file matching that hash in the `unique_chunks/` directory and extract its raw binary data into memory.
4.  **Stitching:** The system appends (writes) that binary data into `restored.txt`. By following the exact sequence of the recipe, the file is perfectly rebuilt byte-by-byte.
