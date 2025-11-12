@echo off
REM ===============================
REM Merge all C++ files from include\ and src\ into one notepad.txt
REM Each file’s name will be added as a header before its contents
REM ===============================

set OUTPUT=merge_cpp_files.txt
set INCLUDE_DIR=include
set SRC_DIR=src

REM Delete old file if exists
if exist "%OUTPUT%" del "%OUTPUT%"

echo Merging files into "%OUTPUT%" ...
echo. >> "%OUTPUT%"

REM Process files in \include\
for %%f in ("%INCLUDE_DIR%\*.cpp" "%INCLUDE_DIR%\*.h" "%INCLUDE_DIR%\*.hpp") do (
    echo ===== [%%~nxf] ===== >> "%OUTPUT%"
    type "%%f" >> "%OUTPUT%"
    echo. >> "%OUTPUT%"
    echo. >> "%OUTPUT%"
)

REM Process files in \src\
for %%f in ("%SRC_DIR%\*.cpp" "%SRC_DIR%\*.h" "%SRC_DIR%\*.hpp") do (
    echo ===== [%%~nxf] ===== >> "%OUTPUT%"
    type "%%f" >> "%OUTPUT%"
    echo. >> "%OUTPUT%"
    echo. >> "%OUTPUT%"
)

echo All done! Files merged into "%OUTPUT%"
pause
