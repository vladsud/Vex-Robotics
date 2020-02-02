@echo off

set TOOLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\bin\Hostx64\x64
set LIBDIR=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\lib\x64
set SDKDIR=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64
set INCL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\include
set INCL2=C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt
set UCRTDIR=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64

"%TOOLDIR%\cl" -c /EHsc -I..\include "-I%INCL%" "-I%INCL2%" MotionTest.cpp Test.cpp
"%TOOLDIR%\link" *.obj /OUT:Test.exe "/LIBPATH:%LIBDIR%" "/LIBPATH:%SDKDIR%" "/LIBPATH:%UCRTDIR%"
del *.obj
