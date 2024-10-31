@echo off
REM 删除并创建目录
rmdir /s /q build
rmdir /s /q output
mkdir build
mkdir output
mkdir output\compressed
mkdir output\decompressed

REM 编译C文件
echo ---Compiling Compressor---
g++ -O2 src\compressor.cpp -o build\compressor.exe
echo ---Compiling Decompressor---
g++ -O2 src\decompressor.cpp -o build\decompressor.exe

echo ---Compressor Output---
build\compressor.exe testcase\debug.txt output\compressed\debug.txt.huff
echo.

echo ---Decompressor Output---
build\decompressor.exe output\compressed\debug.txt.huff output\decompressed\debug.txt
echo.

fc testcase\debug.txt output\decompressed\debug.txt >nul
if errorlevel 1 (
    echo debug.txt: Failed
    echo ---Compressed---
    type output\compressed\debug.txt.huff
    echo.
    echo ---Decompressed---
    type output\decompressed\debug.txt
    echo.
) else (
    echo debug.txt: Passed
)