@echo off
mkdir .\build
gcc .\src\main.c -o .\build\main.exe
for /l %%i in (1,1,8) do (
    echo Testcase #%%i:
    echo OUTPUT
    cat .\data\%%i.in | .\build\main.exe -q
    echo ANSWER
    cat ./data/%%i.ans
    echo:
)