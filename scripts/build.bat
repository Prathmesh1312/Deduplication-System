@echo off
echo Compiling Smart Storage Deduplication System (C Version)...

cd %~dp0\..

gcc -std=c99 -I./include src/main.c src/chunker.c src/sha256.c src/btree.c src/storage.c src/bitvector.c -o dedup.exe

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build succeeded! Run with .\dedup.exe
 is ready.
