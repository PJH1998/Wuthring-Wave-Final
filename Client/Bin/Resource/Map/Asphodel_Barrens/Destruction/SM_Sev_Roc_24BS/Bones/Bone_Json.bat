@echo off
setlocal enabledelayedexpansion

REM 기준 파일 이름
set "source=SM_Sev_Roc_24BS_DM_000_LOD0.json"

REM 현재 디렉터리 기준으로 모든 하위 폴더 순회
for /r %%f in (*.json) do (
    set "filename=%%~nxf"
    set "folder=%%~dpf"

    REM 대상 파일이 source 파일이 아닌 경우에만 처리
    if /I not "!filename!"=="%source%" (
        REM source 파일이 존재하는 경우에만 덮어쓰기
        if exist "!folder!!source!" (
            echo 덮어쓰기: !filename! ← %source%

            REM source 내용을 대상 파일에 덮어쓰기
            copy /Y "!folder!!source!" "%%f" >nul

            REM source 파일 삭제
            del "!folder!!source!"
        ) else (
            echo 건너뜀: !filename! (LOD0 없음)
        )
    )
)

echo 작업 완료!
pause