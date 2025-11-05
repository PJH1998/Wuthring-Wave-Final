@echo off
setlocal enabledelayedexpansion

REM 변경할 문자열을 변수로 설정

set "target=SM_Sev_Tab_NonSonoro_"
set "replacement=SM_Sev_Tab_NonSonoro"

REM 현재 디렉토리 기준으로 작업
for %%F in (*%target%*) do (
    set "oldname=%%F"
    set "newname=!oldname:%target%=%replacement%!"
    ren "%%F" "!newname!"
)

echo 이름 변경 완료.
pause