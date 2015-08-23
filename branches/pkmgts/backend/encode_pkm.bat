@echo off

REM Set this to the relative path to the dir with the .pkm files
set pkmdir=./pkmfiles/

REM Get rid of all old encoded files
cd "%pkmdir%"
del *.proper
cd ..

REM Encode .pkm files
for /f "delims=|" %%f in ('dir /b "%pkmdir%"\') do pkmlib.py "%pkmdir%%%f"

REM pause
exit
