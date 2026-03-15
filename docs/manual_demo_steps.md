# Faculty Demonstration Guide: Smart Storage Deduplication

Follow these steps to demonstrate the core functionality of your system to the faculty.

## 1. Preparation
First, clean up any previous runs to ensure a fresh start. Open your terminal in the root project directory.

```bash
# Clean up any previously generated storage folders or test files
rm -rf unique_chunks
rm -f test_data/demo_original.txt test_data/demo_duplicate.txt test_data/restored_demo.txt
```

## 2. Compile the Project
Use the new build scripts located in the `scripts/` folder to compile the project.

**For macOS / Linux:**
```bash
bash scripts/build.sh
```

**For Windows:**
```powershell
.\scripts\build.bat
```

## 3. Create Test Files
Generate two specific files inside your `test_data/` folder: one original and one that contains duplicate content.

**File 1: `test_data/demo_original.txt`**
Create a file with some unique text.
```bash
echo "This is the first chunk of data. It is unique and will be stored. This is the second chunk of data. It is also unique." > test_data/demo_original.txt
```

**File 2: `test_data/demo_duplicate.txt`**
Create a file that contains the exact same text as File 1.
```bash
echo "This is the first chunk of data. It is unique and will be stored. This is the second chunk of data. It is also unique." > test_data/demo_duplicate.txt
```

## 4. Run the Demonstration

### Step A: Process the Original File
Run the deduplication tool using the `store` command on the first file. It should detect that all chunks are new.
```bash
./dedup store test_data/demo_original.txt
```
**What to tell Faculty:** 
> "The system is chunking the file, calculating SHA-256 hashes, and finding that these hashes don't exist in our B+ Tree index yet. Therefore, it stores them as unique chunks in the `unique_chunks/` folder and generates a '.recipe' file to reconstruct the file later."

### Step B: Process the Duplicate File
Run the tool on the second file. It should detect that they are duplicates.
```bash
./dedup store test_data/demo_duplicate.txt
```
**What to tell Faculty:**
> "Even though this is a different file, the content is identical. The system calculates the same hashes, finds them in the B+ Tree index, and skips storing them, saving 100% of the disk space for this file."

### Step C: File Reconstruction (Restore)
Prove that the deduplication is lossless by rebuilding the original file from the chunks using the generated recipe.
```bash
./dedup restore test_data/demo_original.txt.recipe test_data/restored_demo.txt
```
**What to tell Faculty:**
> "To prove our deduplication didn't corrupt anything, we can reconstruct the exact file natively back from the generated hash recipe."

## 5. Explain the Application Architecture
During the demo, point out these key pillars organized in the (`src/` and `include/`) folders:
1.  **Chunking (`src/chunker.cpp`):** The file is broken into 4KB blocks.
2.  **Hashing (`src/sha256.cpp`):** Each chunk gets a unique SHA-256 'fingerprint'.
3.  **Storage Engine (`src/storage.cpp`):** Physically manages writing raw binary blocks to the disk or reading them during restoration.
4.  **B+ Tree Index (`src/btree.cpp`):** We use a B+ Tree to store and search hashes instantly in memory, ensuring the system remains fast even with millions of chunks!
