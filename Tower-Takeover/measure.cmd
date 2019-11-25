@echo off
touch src\actionsMove.cpp

set start=%time%
make
echo.
echo Start: %start%
echo End  : %time%