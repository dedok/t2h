#include <iostream>
#include "t2h_torrent_cntl.hpp"

static char const * json_config = 
"{\n"
"\"workers\" : \"4\",\n"
"\"server_port\" : \"8080\",\n"
"\"server_addr\" : \"127.0.0.1\",\n" 
"\"doc_root\" : \"test/path\",\n" 
"\"port_start\" : \"6881\",\n"
"\"port_end\" : \"6889\""
"\n}";


static inline void die(std::string const & message, int ec) 
{
	std::cerr << "Error : " << message << ", code " << ec << std::endl;
	std::exit(ec);
}

int main(int argc, char ** argv) 
{
	using namespace t2h_core; 
	setting_manager_ptr sm = setting_manager::shared_manager();
	sm->init_config(json_config);
	if (!sm->config_is_well())
		die(sm->get_last_error(), -1);

	t2h_torrent_core core(sm, "torrent_core");
	if (!core.launch_service())
		die("launch services failed", -1);

	core.add_torrent(argv[1]);
	core.wait_service();
	
	return 0;
}

