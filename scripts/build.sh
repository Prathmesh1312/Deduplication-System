#!/bin/bash
# MacOS/Linux build script

echo "Compiling Smart Storage Deduplication System..."

# Move to the root directory where the script is called from or assume it's run from scripts/
cd "$(dirname "$0")/.."

gcc -std=c99 -I./include src/main.c src/chunker.c src/sha256.c src/btree.c src/storage.c src/bitvector.c -o dedup

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build succeeded! Run with ./dedup"
