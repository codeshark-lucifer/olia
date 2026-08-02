@echo off
set OUTPUT=note.txt

if exist "%OUTPUT%" del "%OUTPUT%"

for /f "delims=" %%F in ('dir /s /b src\*.rs') do (
    >>"%OUTPUT%" echo ============================================================
    >>"%OUTPUT%" echo FILE: %%F
    >>"%OUTPUT%" echo ============================================================
    type "%%F">>"%OUTPUT%"
    >>"%OUTPUT%" echo.
    >>"%OUTPUT%" echo.
)

echo Done.
pause