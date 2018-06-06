@echo off

set COMPILER_EXE=%VULKAN_SDK%\Bin32\glslangValidator.exe

set SRC_DIR=%cd%
set DST_DIR=%cd%

for %%F in (%SRC_DIR%\*.frag) do call %COMPILER_EXE% -V %%F -o %%F.spv
for %%F in (%SRC_DIR%\*.vert) do call %COMPILER_EXE% -V %%F -o %%F.spv

echo Compiling finished
pause
