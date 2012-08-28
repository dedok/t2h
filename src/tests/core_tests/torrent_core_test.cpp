#include "syslogger.hpp"
#include "torrent_core.hpp"
#include "serial_download_tcore_cntl.hpp"

#include <iostream>

static char const * json_config = 
"{\n"
"\"workers\" : \"4\",\n"
"\"server_port\" : \"8080\",\n"
"\"server_addr\" : \"127.0.0.1\",\n" 
"\"doc_root\" : \"test/path\",\n" 
"\"tc_root\" : \"/Users/dedokOne/workspace/src/c++/torrent2http/tc_root\", \n"
"\"tc_port_start\" : \"6881\",\n"
"\"tc_port_end\" : \"6889\", \n"
"\"tc_max_alert_wait_time\" : \"10\", \n" // mseconds
"\"tc_max_async_download_size\" : \"5242880\"" // 5 mb 
"\n}";

static syslogger_settings const log_settings = {
	"com.t2h.TorrentCore",
	"application",
	"application.log"
};

static inline void die(std::string const & message, int ec) 
{
	std::cerr << "Error : " << message << ", code " << ec << std::endl;
	std::exit(ec);
}

int main(int argc, char ** argv) 
{
	using namespace t2h_core; 
	
	LOG_INIT(log_settings)
	
	setting_manager_ptr sm = setting_manager::shared_manager();
	base_torrent_core_cntl_ptr serial_download_cntl(new serial_download_tcore_cntl(sm));
	sm->init_config(json_config);
		
	torrent_core_params params;
	params.setting_manager = sm; 
	params.controller = serial_download_cntl;

	if (!sm->config_is_well())
		die(sm->get_last_error(), -1);

	torrent_core core(params);
	if (!core.launch_service())
		die("launch torrent core services failed", -1);

	int const torrent_id = core.add_torrent(argv[1]);
	if (torrent_id == -1) 
	{ 
		die("add torrent failed", -2);
	}
	else if (torrent_id > 0)
	{
		std::cout << "Torrent added, torrent id : " << torrent_id << std::endl;
		std::cout << "Torrent info : " << core.get_torrent_info(torrent_id) << std::endl;

		int file_id = 0;
		for (;;) {
			std::cout << "Url : " << core.start_torrent_download(torrent_id, file_id) << std::endl;
			std::cin.get(); ++file_id;
		} // ! for
		core.wait_service();
	}
	
	return 0;
}

