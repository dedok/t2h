#include "http_server_core.hpp"
#define DEBUG
#include "syslogger.hpp"

#if defined(__APPLE__)
#include <signal.h>
#endif

#include <iostream>

common::base_service_ptr http_server;

static syslogger_settings const log_settings = {
	"com.t2h.HttpServer", 
	"application",
	"application.log"
};

inline static void die(std::string const & message, int exit_code) 
{
	std::cerr << message << std::endl;
	std::exit(exit_code);
}

static void sig_handler(int signo)
{
	if (signo != SIGINT) 
		return;
	
	if (http_server) { 
		http_server->stop_service();
		http_server->wait_service();
	}
	die("Http server quit.", 0);
}

int main(int argc, char* argv[])
{

#if defined(__APPLE__)
	if (signal(SIGINT, sig_handler) == SIG_ERR) 
	 	die("failed to add signal handler", 1);
#endif

	if (argc != 2) 
		die("Usage: http_server <config_path>", 2); 
	
	LOG_INIT(log_settings)

	try 
	{
		t2h_core::setting_manager_ptr setting_manager = t2h_core::setting_manager::shared_manager();
		setting_manager->load_config(argv[1]);
		http_server.reset(new t2h_core::http_server_core(setting_manager));
		
		if (!http_server->launch_service())
			die("launch failed", -1);
		
		http_server->wait_service();
	} 
	catch (std::exception const & expt) 
	{
		die(expt.what(), -2);
	}

	return 0;
}

