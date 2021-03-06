@echo off

set pwd=%~dp0
set error_message=""
set lib_path=%pwd%lib
set bin_path=%pwd%bin
set full_model_path=%bin_path%\full_model
set full_model_args=--with-config=%pwd%t2h_envt\my_config.json --torrents-dir=%pwd%t2h_envt\torrents
set full_model_command=%full_model_path% %full_model_args%
set build_path=shared_Debug_build

if not exist %pwd%t2h_envt ( 
	error_message="%pwd%t2h_envt not exist ot not well formed"
	goto exit_failure
	)
goto build_and_run_t2h
	
:build_and_run_t2h
	call cd %build_path% && nmake full_model || echo WARNING: build new binary failed.
	cd %pwd%
	if %errorlevel% EQU 0 (
		copy /Y %lib_path%\t2h_core* %bin_path%
		)
	call %full_model_command% || echo ERROR: %full_model_command% failed
goto EOF

:exit_failure
	echo Error detected : %error_message%
	exit /B %errorlevel%

:EOF
	echo.
	echo Exit succeed.
	echo.
	