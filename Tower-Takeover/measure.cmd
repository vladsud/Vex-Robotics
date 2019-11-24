@echo off
touch combined\combined.cpp

set start=%time%
make
echo.
echo Start: %start%
echo End  : %time%