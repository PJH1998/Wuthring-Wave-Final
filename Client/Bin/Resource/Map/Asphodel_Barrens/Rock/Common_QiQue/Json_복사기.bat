@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM 기준 경로 설정
set "ROOT=%~dp0"

REM 하위 2단계 폴더 순회
for /D %%D in ("%ROOT%*") do (
    for /D %%S in ("%%D\*") do (
        REM 해당 폴더에 *_LOD0.json 파일이 있는지 확인
        for %%F in ("%%S\*_LOD0.json") do (
            REM 원본 파일 경로
            set "SRC=%%~fF"
            set "BASE=%%~dpF"
            set "NAME=%%~nxF"

            REM 확장자 분리
            set "EXT=%%~xF"
            set "NAMEONLY=%%~nF"

            REM LOD0 → LOD1, LOD2, LOD3 복사
            for %%N in (1 2 3) do (
                set "NEWNAME=!NAMEONLY:LOD0=LOD%%N!"
                copy "!SRC!" "!BASE!!NEWNAME!!EXT!" >nul
                echo ✔ 복사됨: !NEWNAME!!EXT!
            )
        )
    )
)

pause