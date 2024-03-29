@echo off
py -B -m pytest -p no:cacheprovider
IF /I "%ERRORLEVEL%" NEQ "1" (
    pause
    goto zip
)
goto err

:zip
ROBOCOPY "src/tools" "build/tools"
ROBOCOPY "src" "build" main.py
ROBOCOPY "src" "build" tests.py
cd build
REM py -3-32 -m nuitka --onefile --standalone --include-data-dir="D:\Documents\VS Code\nsui_banner_fixer\build\tools=tools" -o nsui_banner_fixer.exe main.py
py -3-32 -m nuitka --onefile --standalone -o nsui_banner_fixer.exe main.py

echo:
zip nsui_banner_fixer_exe.zip nsui_banner_fixer.exe tools/*
echo zip created
ren main.py nsui_banner_fixer.py
zip nsui_banner_fixer_py.zip nsui_banner_fixer.py tools/*
echo zip py created
ren nsui_banner_fixer.py main.py
goto end

:err
pause

:end
cd "..\src