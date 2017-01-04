@echo off
mkdir ..\build
pushd ..\build
cl -FC -Zi ..\src\stream_record.cpp user32.lib gdi32.lib
popd
