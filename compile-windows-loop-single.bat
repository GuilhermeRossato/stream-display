@echo off
title Setting environment
SET "ENVSCRIPT=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%ENVSCRIPT%" (
	echo Error: Missing Microsoft Visual Studio 2019 enviroment 'vcvars64' script
	echo You may try to compile it but 'cl.exe' might fail
	pause
) else (
	call "%ENVSCRIPT%"
)
WHERE cl
IF %ERRORLEVEL% NEQ 0 (
	echo Error: Could not find 'cl.exe', the Microsoft C/C++ Optimizing Compiler used by this script to compile
	echo Ensure it is installed and available at path
	pause
	exit
)
WHERE timeit
IF %ERRORLEVEL% NEQ 0 (
	SET USETIMEIT=0
) else (
	SET USETIMEIT=1
)
:start
title Compiling and Executing
cl /nologo /MD /Ob0 /O2 ./src/main.cpp /Fe"build/stream.exe" gdi32.lib user32.lib && echo. && "./build/stream.exe" -s 0 0 200 200
: /Dshlwapi /Dole32 /Doleprn /Doleaut32
title Waiting for key to compile and run again
echo.
pause
goto start