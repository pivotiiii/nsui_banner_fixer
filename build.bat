@echo off
ROBOCOPY "src/tools" "build/tools"
ROBOCOPY "src" "build" main.py
ROBOCOPY "src" "build" tests.py
cd build
REM py -3-32 -m nuitka --onefile --standalone --include-data-dir="D:\Documents\VS Code\nsui_banner_fixer\build\tools=tools" -o nsui_banner_fixer.exe main.py
py -3-32 -m nuitka --onefile --standalone -o nsui_banner_fixer.exe main.py
py tests.py
IF /I "%ERRORLEVEL%" NEQ "1" (
    goto zip
)
goto err
:zip
echo:
zip nsui_banner_fixer.zip nsui_banner_fixer.exe tools/*
echo zip created
goto end
:err
pause
:end
cd "..\src