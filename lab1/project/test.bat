@echo off
for /l %%i in (1,1,8) do (
    echo Testcase #%%i:
    echo OUTPUT
    cat ..\data\%%i.in | .\build\lab1_console.exe -q
    echo ANSWER
    cat ..\data\%%i.ans
    echo:
)