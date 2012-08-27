#include "t2h.h"

#include <signal.h>

#include <map>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

/** 
 * Helpers declaraion 
 */
void sig_handler(int signo);
void die(std::string const & message, int error_code = 1); 

/** 
 * Type declarations 
 */
struct t2h_handle_wrapper {
	t2h_handle_t handle;
	std::map<int, std::string> info_map;
};

struct program_options{
	program_options(boost::program_options::variables_map & vm) 
		: is_valid(false), config_path(), torrent_dir() 
	{   
		if (vm.count("with-config")) 
			config_path = vm["with-config"].as<std::string>();
		if (vm.count("torrents-dir")) 
			torrent_dir = vm["torrent-dir"].as<std::string>();
	}
	
	bool is_valid;
	std::string config_path;
	std::string torrent_dir;
};

/** 
 * Main functionality 
 */
void add_torrents_from_directory(t2h_handle_wrapper & handle, program_options const & options) 
{
	using namespace boost::filesystem;
	directory_iterator it(options.torrent_dir), end;
	for (;it != end; ++it) {
		int torrent_id = 
			t2h_add_torrent(handle.handle, it->path().string().c_str());
		if (torrent_id != -1)
			handle.info_map[torrent_id] = "";	
		else
			std::cout << "Warnign : can not add torrent at path : " << it->path().string().c_str();
	}
}

void start_download_all_torrents(t2h_handle_wrapper & handle) 
{
	for (std::map<int, std::string>::iterator it = handle.info_map.begin(), end = handle.info_map.end();
		it != end;
		++it) 
	{
		it->second = t2h_start_download(handle.handle, it->first, 0);
	} 
}

inline program_options get_options(int argc, char ** argv) 
{
	namespace po = boost::program_options;
	
	po::variables_map vm;	
	program_options options(vm);	
	std::string default_torrent_dir = "";
	po::options_description desc("Allowed options");

	desc.add_options()
	    ("help", "produce help message")
		    ("with-config", po::value<std::string>(), "core config path")
			("torrents-dir", po::value<std::string>(&default_torrent_dir)->default_value(""), "directory with .torrent files [OPTIOANAL]")
			;

	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);  

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		die("", 0);
	}

	return options; 
}

/** 
 * Entry point 
 */

static t2h_handle_wrapper core_handle;

int main(int argc, char ** argv) 
{
	if (signal(SIGINT, sig_handler) == SIG_ERR) 
	 	die("failed to add signal handler", 1);

	program_options options = get_options(argc, argv);
	
	core_handle.handle = t2h_init(options.config_path.c_str());
	if (!core_handle.handle)
		die("failed to start torrent to http core", 2);

	add_torrents_from_directory(core_handle, options);	
	start_download_all_torrents(core_handle);	
	
	t2h_wait(core_handle.handle);

	return 0;
}

/** 
 * Helpers implementation 
 */

void sig_handler(int signo)
{
	if (signo != SIGINT) 
		return;
	t2h_close(core_handle.handle);
}

void die(std::string const & message, int error_code) 
{
	if (!message.empty())
		std::cerr << "Fatal error : " << message << std::endl;
	std::exit(error_code);
}

