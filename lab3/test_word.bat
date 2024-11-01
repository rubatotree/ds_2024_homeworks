@echo off
REM 删除并创建目录
rmdir /s /q build
rmdir /s /q output
mkdir build
mkdir output
mkdir output\compressed
mkdir output\decompressed

echo ---Compiling Compressor---
g++ -O2 src\compressor.cpp -o build\compressor.exe
if %errorlevel% equ 0 (
    echo ---Compiling Decompressor---
    g++ -O2 src\decompressor.cpp -o build\decompressor.exe
    for %%t in (0 1 2 3 4 5 6 7 8 9 10 20 40 80 200 500) do (
        build\compressor.exe testcase\2.hamlet.txt output\compressed\2.hamlet.%%t.txt.huff -w %%t >nul
        build\decompressor.exe output\compressed\2.hamlet.%%t.txt.huff output\decompressed\2.hamlet.%%t.txt >nul

        fc testcase\2.hamlet.txt output\decompressed\2.hamlet.%%t.txt >nul
        if %errorlevel% equ 0 (
            echo Threshold %%t Passed
            python3 src\utils\calc_compress_radio.py testcase\2.hamlet.txt output\compressed\2.hamlet.%%t.txt.huff
        ) else (
            echo Threshold %%t Failed
        )
    )

    build\compressor.exe testcase\2.hamlet.txt output\compressed\2.hamlet.inf.txt.huff >nul
    build\decompressor.exe output\compressed\2.hamlet.inf.txt.huff output\decompressed\2.hamlet.inf.txt >nul

    fc testcase\2.hamlet.txt output\decompressed\2.hamlet.inf.txt >nul
    if %errorlevel% equ 0 (
        echo Threshold inf Passed
        python3 src\utils\calc_compress_radio.py testcase\2.hamlet.txt output\compressed\2.hamlet.inf.txt.huff
    ) else (
        echo Threshold inf Failed
    )
) else (
    echo Compile error.
)