@echo off
pushd ..\
call vendor\premake\premake5.exe vs2026
popd
PAUSE
