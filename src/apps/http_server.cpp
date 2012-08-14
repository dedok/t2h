#include "t2h_http_server_cntl.hpp"
#define DEBUG
#include "syslogger.hpp"

#include <iostream>

static syslogger_settings const log_settings = {
	"com.t2h.HttpServer", 
	"application",
	"/Users/dedokOne/application.log"
};


inline static void die(std::string const & message, int exit_code) 
{
	std::cerr << message << std::endl;
	std::exit(exit_code);
}

int main(int argc, char* argv[])
{
	if (argc != 2) 
		die("Usage: http_server <config_path>", 1); 
	
	LOG_INIT(log_settings)

	try 
	{
		t2h_core::setting_manager_ptr setting_manager = t2h_core::setting_manager::shared_manager();
		setting_manager->load_config(argv[1]);
		common::base_service_ptr http_server(
			new t2h_core::http_server_cntl("t2h_http_server", setting_manager));
		
		if (!http_server->launch_service())
			die("launch failed", -1);
		
		for (;;) 
			{ 
			sleep(10); 
			}
		http_server->wait_service();
	} 
	catch (std::exception const & expt) 
	{
		die(expt.what(), -2);
	}

	return 0;
}

