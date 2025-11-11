@echo off
setlocal enabledelayedexpansion

:: --- (Configuration) ---
:: Set the strings you want to find and replace.
set "target=SM_Tab_Com2_Bri"
set "replacement=SM_Com2_Bri"

:: --- (Execution) ---

:: Change the current directory to the script's location.
:: This makes all paths stable.
cd /d %~dp0

echo ======================================================
echo  Rename Operation Started...
echo  Target: *%target%*
echo  Replacement: %replacement%
echo ======================================================
echo.

REM --- 1. Rename Files (Recursive) ---
echo [Searching for FILES...]
FOR /R . %%F IN (*%target%*) DO (
    set "oldname=%%~nxF"
    set "newname=!oldname:%target%=%replacement%!"
    
    IF "%%~nxF" NEQ "!newname!" (
        echo [File] "%%F" --^> "!newname!"
        ren "%%F" "!newname!"
    )
)

echo.
echo [Searching for FOLDERS (Deepest first)...]
REM --- 2. Rename Folders (Recursive, Deepest first) ---

FOR /F "delims=" %%D IN ('DIR /B /S /AD "*%target%*" | SORT /R') DO (
    set "oldname=%%~nxD"
    set "newname=!oldname:%target%=%replacement%!"
    
    IF "%%~nxD" NEQ "!newname!" (
        echo [Folder] "%%D" --^> "!newname!"
        ren "%%D" "!newname!"
    )
)

echo.
echo ======================================================
echo  All tasks complete.
echo ======================================================

endlocal
pause