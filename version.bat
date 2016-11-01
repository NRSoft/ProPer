:: Windows batch file
::
:: Don't forget to adjust PATH to point to the git binary location
::  e.g. set PATH=%PATH%;"C:\Program Files\Git\bin"
::  or (better) do it by changing user environment variable PATH
::  in ControlPanel/System/AdvancedSystemSettings/EnvironmentVariables
::
@echo off

:: change major version number HERE if needed
::  and reset the minor number in "version.h" to 0
set major=0

:: check if there are any changes at all
set status=stat.diff
git status -s > %status%
for /f %%i in ("%status%") do set size=%%~zi
if %size% EQU 0 exit

:: check if version.h has already been changed
set header=version.h
git diff %header% > %status%
for /f %%i in ("%status%") do set size=%%~zi
if %size% NEQ 0 exit

:: if not changed yet: increment minor version
set /p var=<%header%
set /a var=%var:~22%+1
echo #define VERSION_MINOR %var% >%header%
echo #define VERSION_MAJOR %major% >>%header%
echo #define VERSION_DATE "%date:~4,10%">>%header%
