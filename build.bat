@echo off

if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
)

cl /nologo /Zi twitch_api.c test.c /link /out:test.exe