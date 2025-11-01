@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM /S (하위 폴더) /B (경로만)
REM 2^>nul : "파일을 찾을 수 없습니다" 오류 숨김
REM | sort /r : 결과를 역순으로 정렬 (가장 깊은 경로가 먼저 오도록 함)
echo [!] "파일" 및 "폴더" 이름 변경 작업을 시작합니다...
echo.

REM dir /s /b 는 파일과 폴더를 모두 찾습니다.
for /f "delims=" %%F in ('dir /s /b *SM_Sev_Roc_24BS_DM_* 2^>nul ^| sort /r') do (
    
    REM %%~nxF는 "파일.확장자" 또는 "폴더명" (경로 제외)
    set "oldname=%%~nxF"
    
    REM 문자열 치환
    set "newname=!oldname:SM_Sev_Roc_24BS_DM_=SM_Sev_Roc_24BS_!"
    
    REM oldname과 newname이 다를 경우에만 (즉, 치환이 일어난 경우에만) 실행
    if "!oldname!" NEQ "!newname!" (
        echo [변경] "%%F"
        echo    -> "!newname!"
        
        REM "ren [전체 경로] [새 이름]"
        ren "%%F" "!newname!"
    )
)

echo.
echo --- 모든 이름 변경 완료 ---
pause