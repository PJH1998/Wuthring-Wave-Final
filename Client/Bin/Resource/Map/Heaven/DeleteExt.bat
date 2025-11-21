@echo off
setlocal

rem ==== 확장자 인자 체크 ====
if "%~1"=="" (
    echo        %~nx0 .png
    goto :eof
)

rem ==== 확장자 앞에 . 없으면 붙여주기 ====
set "EXT=%~1"
if not "%EXT:~0,1%"=="." set "EXT=.%EXT%"

echo.
echo 현재 폴더와 모든 하위 폴더에서 "%EXT%" 파일을 삭제합니다.
echo 취소하려면 Ctrl + C 누르고, 계속하려면 아무 키나 누르세요...
pause >nul

rem ==== 재귀적으로 해당 확장자 삭제 ====
for /r "%cd%" %%F in (*%EXT%) do (
    echo 삭제: "%%F"
    del /f /q "%%F"
)

echo.
echo 작업 완료!
endlocal