@echo off
echo Generating icon resource...
windres baka.rc -O coff -o baka.res
if errorlevel 1 goto :error

echo Compiling DF9 compiler with icon...
g++ -std=c++17 -static -static-libstdc++ -static-libgcc df9.cpp baka.res -o df9.exe
if errorlevel 1 goto :error

echo Success! df9.exe is ready with baka.ico
pause
goto :eof

:error
echo Failed to compile df9.exe
pause