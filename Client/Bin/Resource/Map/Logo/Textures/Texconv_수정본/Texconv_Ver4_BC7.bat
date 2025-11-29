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
    
    REM findstr가 문자열을 찾지 못했을 때(errorlevel 1)만 변환 실행
    if !errorlevel! NEQ 0 (
        set "OUT=%%~dpF"
        call set "OUT=!OUT:~0,-1!"

        REM --- 텍스처 유형별 포맷 결정 ---
        
        REM [변경] 기본 포맷: BC3 -> BC7 (식별 안 된 텍스처도 고품질로 변환)
        set "FORMAT=BC7_UNORM"

        REM 1. _NB (노말 맵) -> BC5 유지
        if /I "!NAME:~-3!" == "_NB" (
            set "FORMAT=BC5_UNORM"
        ) else (
            REM 2. _DB (디퓨즈/베이스) -> BC7 로 변경
            if /I "!NAME:~-3!" == "_DB" (
                set "FORMAT=BC7_UNORM"
            ) else (
                REM 3. _N (노말 맵) -> BC5 유지
                if /I "!NAME:~-2!" == "_N" (
                    set "FORMAT=BC5_UNORM"
                ) else (
                    REM 4. _D (디퓨즈) -> BC7 로 변경
                    if /I "!NAME:~-2!" == "_D" (
                        set "FORMAT=BC7_UNORM"
                    )
                )
            )
        )

        REM --- 변환 실행 ---
        echo ▶ [!FORMAT!] 변환: "!FULL!"
        
        REM BC7 변환은 시간이 좀 더 걸리므로 -gpu 0 옵션을 주면 명시적으로 GPU 가속을 사용합니다 (가능한 경우)
        %TEXCONV% -y -f !FORMAT! -m 0 -o "!OUT!" "!FULL!"
        
    ) else (
        REM findstr가 문자열을 찾았을 때 (errorlevel 0)
        echo ▶ [제외] DefaultColorWhite 파일: "!FULL!"
    )
)

pause