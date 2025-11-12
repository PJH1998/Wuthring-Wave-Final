@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM 현재 스크립트 위치 기준 한 칸 상위 폴더로 이동
cd /d "%~dp0\.."

REM texconv.exe 경로 설정
set TEXCONV="%~dp0texconv.exe"

for /R %%F in (*.png *.jpg) do (
    set "FULL=%%~F"
    set "NAME=%%~nF"

    REM --- 예외 처리: 파일 이름에 DefaultColorWhite가 포함되어 있는지 확인 ---
    echo !NAME! | findstr /I "DefaultColorWhite" >nul
    set "SKIP=!errorlevel!"

    if "!SKIP!"=="1" (
        set "OUT=%%~dpF"
        call set "OUT=!OUT:~0,-1!"

        REM 기본 포맷: BC3 (알파 포함 컬러)
        set "FORMAT=BC3_UNORM"

        REM 접미사 확인
        if /I "!NAME:~-3!"=="_NB" (
            set "FORMAT=R8G8B8A8_UNORM"
        ) else if /I "!NAME:~-3!"=="_DB" (
            set "FORMAT=BC3_UNORM"
        ) else if /I "!NAME:~-2!"=="_N" (
            set "FORMAT=R8G8B8A8_UNORM"
        ) else if /I "!NAME:~-2!"=="_D" (
            set "FORMAT=BC3_UNORM"
        )

        echo ▶ [!FORMAT!] 변환: "!FULL!"
        %TEXCONV% -y -f !FORMAT! -m 0 -o "!OUT!" "!FULL!"
    ) else (
        echo ▶ [제외] DefaultColorWhite 파일: "!FULL!"
    )
)

pause