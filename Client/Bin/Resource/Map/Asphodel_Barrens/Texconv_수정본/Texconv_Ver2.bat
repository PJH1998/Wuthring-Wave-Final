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
    set "OUT=%%~dpF"
    call set "OUT=!OUT:~0,-1!"

    REM --- 텍스처 유형별 포맷 결정 (가장 긴 접미사부터 확인) ---
    
    REM 기본 포맷: BC3 (알파 포함 컬러)
    set "FORMAT=BC3_UNORM"

    REM 1. _NB (3글자) 확인
    if /I "!NAME:~-3!" == "_NB" (
        set "FORMAT=BC5_UNORM"
    ) else (
        REM 2. _DB (3글자) 확인
        if /I "!NAME:~-3!" == "_DB" (
            set "FORMAT=BC3_UNORM"
        ) else (
            REM 3. _N (2글자) 확인
            if /I "!NAME:~-2!" == "_N" (
                set "FORMAT=BC5_UNORM"
            ) else (
                REM 4. _D (2글자) 확인
                if /I "!NAME:~-2!" == "_D" (
                    set "FORMAT=BC3_UNORM"
                )
            )
        )
    )

    REM --- 변환 실행 ---
    echo ▶ [!FORMAT!] 변환: "!FULL!"
    %TEXCONV% -y -f !FORMAT! -m 0 -o "!OUT!" "!FULL!"
)

pause