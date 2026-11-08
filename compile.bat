@echo off
setlocal

set /p name=Input File:

if not defined name (
    echo No input file.
    exit /b 1
)
if not exist "%name%" (
    echo File not found: "%name%"
    exit /b 1
)

for %%F in ("%name%") do set "base=%%~dpnF"

echo [1/3] df9 "%name%" ^> "%base%.s"
df9.exe "%name%" > "%base%.s"
if errorlevel 1 (
    echo DF9 compile failed.
    exit /b 1
)

echo [2/3] gcc "%base%.s" -o "%base%.exe"
rem 针对 Windows 的原生 gcc，不需要 -no-pie，保留 -lpthread -lm
gcc "%base%.s" -o "%base%.exe" -lpthread -lm
if errorlevel 1 (
    echo gcc failed.
    exit /b 1
)

echo [3/3] Run "%base%.exe"
echo ----------------------------------------
"%base%.exe"
echo ----------------------------------------