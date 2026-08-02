@echo off
setlocal

:: ===== Change these if needed =====
set MSYS=D:\msys64\ucrt64
set DEST=D:\Program Education\rust\olia

echo Creating folders...
mkdir "%DEST%\include\SDL3" 2>nul
mkdir "%DEST%\lib" 2>nul
mkdir "%DEST%\bin" 2>nul

echo Copying SDL3 headers...
xcopy "%MSYS%\include\SDL3\*" "%DEST%\include\SDL3\" /E /I /Y

echo Copying SDL3 libraries...
copy "%MSYS%\lib\libSDL3.dll.a" "%DEST%\lib\" /Y
copy "%MSYS%\lib\libSDL3.a" "%DEST%\lib\" /Y

echo Copying SDL3 runtime...
copy "%MSYS%\bin\SDL3.dll" "%DEST%\bin\" /Y

echo.
echo ============================
echo SDL3 copied successfully!
echo ============================

pause