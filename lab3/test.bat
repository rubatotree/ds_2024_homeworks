@echo off
REM 删除并创建目录
rmdir /s /q build
rmdir /s /q output
mkdir build
mkdir output
mkdir output\compressed
mkdir output\decompressed

REM 编译C文件
g++ -O2 src\compressor.cpp -o build\compressor.exe
g++ -O2 src\decompressor.cpp -o build\decompressor.exe

REM 遍历testcase目录中的文件
for %%f in (testcase\*) do (
    build\compressor.exe testcase\%%~nxf output\compressed\%%~nxf.huff >nul
    build\decompressor.exe output\compressed\%%~nxf.huff output\decompressed\%%~nxf >nul
    fc testcase\%%~nxf output\decompressed\%%~nxf >nul
    if errorlevel 1 (
        echo %%~nxf: Failed
    ) else (
        echo %%~nxf: Passed
    )
    python3 src\utils\calc_compress_radio.py testcase\%%~nxf output\compressed\%%~nxf.huff
)