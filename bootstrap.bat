@echo off

::
:: bootstrap t2h lib for windows platforms, for details use bootstrap.bat --help
:: Soshnikov Vasiliy<dedok.mad@gmail.com>
::

:: Glob vars

set build_dir=
set bin_dir=%CD%\bin
set lib_dir=%CD%\lib
set error_message=
set program_name=%0
set cmake_source=%CD%\src
set build_type=Debug
set out_build_type=shared
set cmake_generator_type=NMake Makefiles
set cmake_extra_args=-DT2H_CORE_SHARED:BOOL=TRUE

:: Entry point

echo %program_name%

for %%A in (%*) do (
    if "%%A" == "--static" ( 
		set out_build_type=static
		set cmake_extra_args=
		)
	if "%%A" == "--debug" set build_type=Debug
	if "%%A" == "--release" set build_type=Release
	if "%%A" == "--rel-with-deb-info" set build_type=RelWithDebInfo
	if "%%A" == "--help" goto usage
	)

set build_dir=%CD%\%build_type%_build

if exist %bin_dir% rmdir /s /q %bin_dir% 
mkdir %bin_dir%

if exist %lib_dir% rmdir /s /q %lib_dir% 
mkdir %lib_dir%

if exist %build_dir% rmdir /s /q %build_dir% 
mkdir %build_dir%

echo.
echo Current options : 
echo build_dir [%build_dir%] 
echo build_type [%build_type%] 
echo lib type [%out_build_type%]
echo cmake source [%cmake_source%]
echo.

goto run_cmake

:: bootstrap helpers

:run_cmake
	set cmake_ending_args=-G"%cmake_generator_type%" -B%build_dir% -DCMAKE_BUILD_TYPE=%build_type%
	cmake %cmake_ending_args% %cmake_extra_args% %cmake_source%
	if %errorlevel% EQU 0 goto end
	set error_message=cmake command failed
goto failed_exit

:usage
	echo.
	echo %program_name% "[OPT --static(default shared)] [OPT --(debug|release|rel-with-deb-info)]"
	echo.
	echo Description
	echo.
	echo    Prepareing t2h lib to build for Windows platforms. 
	echo    %program_name% can failed with difference error, all bad codes not equ to 0.
	echo.
	echo    NOTE Make sure You setup MSVC envt. and t2h envt. before run the %program_name%,
	echo         otherwise the %program_name% should fail.
	echo    NOTE For extra(including setup envt.) help see the %CD%\README.txt
	echo    NOTE %program_name% must launching only from t2h git repo
	echo.
	echo Optinons
	echo.
	echo    --help                                                 Print usage message and exit with code 0
	echo    --static / --shared(default)                           The library out type
	echo    --debug(default) / --release / --rel-with-deb-info     Build type
	echo.
	echo.
goto EOF
	
:failed_exit
	echo.
	echo %program_name% fatal error : error level %errorlevel%, message %error_message% .
	echo.
	exit /B %errorlevel%
goto EOF

:end
	echo.
	echo To start build type ">cd %build_dir% && nmake [OPT TARGET]"
	echo %program_name% is done.
	echo.
	exit /B 0
goto EOF

:EOF
