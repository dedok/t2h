#include "syslogger.hpp"
#include "torrent_core.hpp"
#include "sequential_torrent_controller.hpp"

#include <iostream>
#include <boost/filesystem.hpp>

static char const * json_config = 
"{\n"
"\"workers\" : \"4\",\n"
"\"server_port\" : \"8080\",\n"
"\"server_addr\" : \"127.0.0.1\",\n" 
"\"doc_root\" : \"tc_root\",\n" // not needed for this test 
"\"tc_root\" : \"tc_root\", \n"
"\"tc_port_start\" : \"6881\",\n"
"\"tc_port_end\" : \"6889\", \n"
"\"tc_max_alert_wait_time\" : \"10\"" // mseconds
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

#define PRINT_ std::cout << __FUNCTION__ << " " 

class event_handler : public t2h_core::torrent_core_event_handler {
public :

	event_handler() { }
	virtual ~event_handler() { }	

	virtual void on_file_remove(std::string const & file_path) 
	{
		PRINT_ << "file path : " << file_path << std::endl;
	}
	
	virtual void on_file_add(std::string const & file_path, boost::int64_t file_size) 
	{
#if PRINT_ADD_EVENT
		PRINT_ << "File path : " << file_path << std::endl 
			<< "File size : " << file_size << std::endl;
#endif
	}

	virtual void on_pause(std::string const & file_path) 
	{
		PRINT_ << "File path : " << file_path << std::endl;
	}

	virtual void on_file_complete(std::string const & file_path, boost::int64_t avaliable_bytes) 
	{
		PRINT_ << "File path : " << file_path << std::endl 
			<< "Avaliable_bytes : " << avaliable_bytes << std::endl;
	}

	virtual void on_progress_update(std::string const & file_path, boost::int64_t avaliable_bytes) 
	{
		boost::system::error_code ec;
		PRINT_ << "Real file size : " << boost::filesystem::file_size(file_path, ec) << std::endl 
				<< "File path : " << file_path << " avaliable_bytes : " << avaliable_bytes << std::endl;
	}

};

int main(int argc, char ** argv) 
{
	using namespace t2h_core; 
	
	if (argc <= 1) return EXIT_FAILURE;

	LOG_INIT(log_settings)
	
	setting_manager_ptr sm = setting_manager::shared_manager();
	base_torrent_core_cntl_ptr stc(new sequential_torrent_controller());
	torrent_core_event_handler_ptr ev(new event_handler());

	sm->init_config(json_config);
		
	torrent_core_params params;
	params.setting_manager = sm; 
	params.controller = stc;
	params.event_handler = ev;

	if (!sm->config_is_well())
		die(sm->get_last_error(), -1);

	torrent_core core(params);
	if (!core.launch_service())
		die("launch torrent core services failed", -1);

	std::size_t const torrent_id = core.add_torrent(argv[1]);
	if (torrent_id == torrent_core::invalid_torrent_id) 
	{ 
		die("add torrent failed", -2);
	}
	else
	{
		std::cout << "Torrent added, torrent id : " << torrent_id << std::endl;
		std::cout << "Torrent info : " << core.get_torrent_info(torrent_id) << std::endl;

		int file_id = 0;
		for (;;) {
			std::cout << "Start download, path to : " << core.start_torrent_download(torrent_id, file_id) << std::endl;
			std::cin.get(); ++file_id;
		} // ! for
	}
	core.wait_service();
	std::cout << "End..." << std::endl;
	return 0;
}

