@echo off
setlocal enabledelayedexpansion

REM 변경할 문자열을 변수로 설정
set "target=SM_Tab_Com2_Bri"
set "replacement=SM_Com2_Bri"

REM --- 1. 폴더 이름 변경 ---
echo [폴더 이름 변경 시작...]
REM /D 플래그: 디렉토리(폴더)만 순회합니다.
for /D %%D in (*%target%*) do (
    set "oldname=%%D"
    set "newname=!oldname:%target%=%replacement%!"
    echo [폴더] "!oldname!" --^> "!newname!"
    ren "%%D" "!newname!"
)

echo.
echo [파일 이름 변경 시작...]
REM --- 2. 파일 이름 변경 ---
for %%F in (*%target%*) do (
    set "oldname=%%F"
    set "newname=!oldname:%target%=%replacement%!"
    echo [파일] "!oldname!" --^> "!newname!"
    ren "%%F" "!newname!"
)

echo.
echo 모든 이름 변경 완료.
pause