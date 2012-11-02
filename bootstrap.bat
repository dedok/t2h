@echo off
::
:: bootstrap t2h lib for windows platforms, for details use bootstrap.bat --help
:: Soshnikov Vasiliy<dedok.mad@gmail.com>
::

:: Glob vars

set build_dir=
set want_eiw=no
set want_clean=no
set error_message=
set program_name=%0
set bin_dir=%CD%\bin
set lib_dir=%CD%\lib
set build_type=Debug
set cmake_source=%CD%\src
set out_build_type=shared
set export_vars_script=%CD%\export_vars.bat
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
	if "%%A" == "--clean" set want_clean=yes
	if "%%A" == "--eiw" set want_eiw=yes
	)

set build_dir=%CD%\%out_build_type%_%build_type%_build

if "%want_clean%" == "yes" goto clean 

echo.
echo Current options : 
echo build_dir [%build_dir%] 
echo build_type [%build_type%] 
echo lib type [%out_build_type%]
echo cmake source [%cmake_source%]
echo.

if not exist mkdir %lib_dir%
if not exist mkdir %build_dir%
if not exist mkdir %bin_dir%

goto run_cmake

:: bootstrap helpers

:run_cmake
	set cmake_extra=-DT2H_INT_WORKAROUND:BOOL=TRUE
	if exist %export_vars_script% ( 
		:: NOTE export_vars_script_output must echo cmake args.
		if exist %CD%\export_vars_script_output del %CD%\export_vars_script_output
		call %export_vars_script% >> %CD%\export_vars_script_output
		set /p cmake_envt_args=< %CD%\export_vars_script_output
		del export_vars_script_output
		echo.
		echo Found "%export_vars_script%", 
		echo cmake envt. args set to "%cmake_envt_args%" %cmake_extra%
		echo.
	)
	if "%want_eiw%"=="yes" set cmake_extra=-DT2H_INT_WORKAROUND:BOOL=TRUE
	set cmake_ending_args=-G"%cmake_generator_type%" -B%build_dir% -DCMAKE_BUILD_TYPE=%build_type%
	cmake %cmake_ending_args% %cmake_extra_args% -H%cmake_source% %cmake_envt_args% %cmake_extra%
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
	echo         otherwise the %program_name% migth fail.
	echo    NOTE For extra(including setup envt.) help see the %CD%\README.txt
	echo    NOTE %program_name% must launching only from t2h git repo root
	echo.
	echo. Envt.
	echo.
	echo     BOOST_ROOT - path to boost root, example : set BOOST_ROOT=C:/boost_1_47/
	echo     LIBTORRENT_ROOT - path to libtorrent root, example : set LIBTORRENT_ROOT=C:/libtorrent_0_16_3/
	echo     As alternative, set BOOST_ROOT and LIBTORRENT_ROOT inside file '%export_vars_script%' and do 
	echo     at the EOF "echo -DBOOST_ROOT=path-to-root -DLIBTORRENT_ROOT=path-to-root"
	echo.
	echo Optinons
	echo.
	echo    --eiw                                                  On int workaraund
	echo    --help                                                 Print usage message and exit with code 0
	echo    --static / --shared(default)                           The library out type
	echo    --debug(default) / --release / --rel-with-deb-info     Build type
	echo    --clean                                                clean all prev. build(s) data
	echo.
goto EOF

:clean
	if exist %bin_dir% rmdir /s /q %bin_dir% 
	if exist %lib_dir% rmdir /s /q %lib_dir% 
	if exist %build_dir% rmdir /s /q %build_dir% 
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
