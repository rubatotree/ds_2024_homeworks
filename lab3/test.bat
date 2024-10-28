@echo off
REM 删除并创建目录
rmdir /s /q build
rmdir /s /q output
mkdir build
mkdir output
mkdir output\compressed
mkdir output\decompressed

REM 编译C文件
gcc -O2 src\compressor.c -o build\compressor.exe
gcc -O2 src\decompressor.c -o build\decompressor.exe

REM 遍历testcase目录中的文件
for %%f in (testcase\*) do (
    .\build\compressor.exe testcase\%%~nxf output\compressed\%%~nxf
    .\build\decompressor.exe output\compressed\%%~nxf output\decompressed\%%~nxf
    fc testcase\%%~nxf output\decompressed\%%~nxf >nul
    if errorlevel 1 (
        echo %%~nxf: Failed
    ) else (
        echo %%~nxf: Passed
    )
)