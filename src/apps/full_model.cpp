#if defined(T2H_CORE_SHARED)
#	define T2H_IMPORT
#endif // T2H_CORE_SHARED

#include "t2h.h"

#if defined(UNIX) || defined(__APPLE__)
#include <signal.h>
#endif

#include <map>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_array.hpp>
#include <boost/program_options.hpp>

/** 
 * Helpers declaraion 
 */
void sig_handler(int signo);
void die(std::string const & message, int error_code = 1); 

/** 
 * Type declarations 
 */
typedef boost::shared_array<char> shared_bytes_type;
typedef std::map<int, shared_bytes_type> info_map_type;

struct t2h_handle_wrapper {
	t2h_handle_t handle;
	boost::mutex im_lock;
	info_map_type info_map;
};

static t2h_handle_wrapper core_handle;
static bool volatile sig_exit = false;

struct program_options{
	bool is_valid;
	std::string config_path;
	std::string torrent_dir;
};

/** 
 * Main functionality 
 */
void console_controller(); 

void add_torrents_from_directory(t2h_handle_wrapper & handle, program_options const & options); 

program_options get_options(int argc, char ** argv); 

void do_action_dispatch_vm(boost::program_options::variables_map const & vm, bool & exit); 

void dispatch_text_command(char const * ibuf, std::size_t ibuf_size, bool & exit); 

/** 
 * Entry point 
 */

int main(int argc, char ** argv) 
{
	try 
	{
#if defined(UNIX) || defined(__APPLE__)
		if (signal(SIGINT, sig_handler) == SIG_ERR) 
		 	die("failed to add signal handler", 1);
#endif
		program_options options = get_options(argc, argv);
	
		core_handle.handle = t2h_init(options.config_path.c_str());
		if (!core_handle.handle)
			die("failed to start torrent to http core", 2);

		add_torrents_from_directory(core_handle, options);

		boost::thread console_controller_loop(&console_controller);		
		t2h_wait(core_handle.handle);
		console_controller_loop.join();
	} 
	catch (std::exception const & expt) 
	{ 
		die(expt.what(), 3);
	}

	return 0;
}

/** 
 * Helpers implementation 
 */

void console_controller() 
{
	std::size_t const ibuf_size = 256;
	char ibuf[ibuf_size];
	for (bool exit_state = false; !exit_state || !sig_exit; std::memset(ibuf, ibuf_size, 0)) {
		std::cout << "Waiting next command..." << std::endl;
		std::cin.getline(ibuf, ibuf_size);
		try 
		{
			dispatch_text_command(ibuf, ibuf_size, exit_state);	
		} 
		catch (boost::program_options::unknown_option const & expt) 
		{
			std::cout << "Option error : " << expt.what() << std::endl;
		}
	} // !loop	
	std::cout << "Exit from console_controller" << std::endl;
}

void add_torrents_from_directory(t2h_handle_wrapper & handle, program_options const & options) 
{
	using namespace boost::filesystem;
	
	boost::system::error_code error_code;
	if (!exists(options.torrent_dir, error_code))
		die(std::string("torrent directory not exist " + options.torrent_dir), 1);
	
	std::cout << "Read torrents directory : " << options.torrent_dir << std::endl;
	directory_iterator it(options.torrent_dir), end;
	for (;it != end; ++it) {
		int torrent_id = t2h_add_torrent(handle.handle, it->path().string().c_str());
		if (torrent_id != INVALID_TORRENT_ID) 
		{
			boost::lock_guard<boost::mutex> guard(handle.im_lock);
			handle.info_map[torrent_id] = shared_bytes_type();
			std::cout << "Adding torrent by path : " << 
				it->path().string() << " , with id " << torrent_id << std::endl;
		}
		else
			std::cout << "Warning : can not add torrent at path : " << it->path().string().c_str();
	}
}

program_options get_options(int argc, char ** argv) 
{
	namespace po = boost::program_options;
	program_options options;
	
	po::variables_map vm;	
	po::options_description desc("Allowed options");
	
	try 
	{
		desc.add_options()
		    ("help", "produce help message")
			    ("with-config", po::value<std::string>(&options.config_path), "core config path")
				("torrents-dir", po::value<std::string>(&options.torrent_dir), "directory with .torrent files")
				;

		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);  
	
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			die("", 0);
		}
	}
	catch (std::exception const & expt) 
	{	
		die(expt.what(), -1);
	}
	catch (...) 
	{
		die("error in parsing command line options", -2);
	}

	return options; 
}

void do_action_dispatch_vm(boost::program_options::variables_map const & vm, bool & exit) 
{
	boost::lock_guard<boost::mutex> guard(core_handle.im_lock);
	if (vm.count("tor_stop")) { 
		std::cout << "Stoping..." << std::endl;
		t2h_stop_download(core_handle.handle, vm["tor_stop"].as<int>());
	}
	else if (vm.count("tor_pause")) { 
		std::cout << "Pausing..." << std::endl;
		t2h_paused_download(core_handle.handle, vm["tor_pause"].as<int>(), 1);	
	}
	else if (vm.count("tor_remove")) { 
		std::cout << "Removing..." << std::endl;
		t2h_delete_torrent(core_handle.handle, vm["tor_remove"].as<int>());
	}
	else if (vm.count("tor_start_download")) {
		int const torrent_id = vm["tor_start_download"].as<int>();	
		std::cout << "Staring download..." << std::endl;
		if (!core_handle.info_map.count(torrent_id)) {
			std::cout << "Failed to start download : no such torrent id" << std::endl;
			return;
		}
		char * mem = t2h_start_download(core_handle.handle, torrent_id, 1);
		if (!mem) {
			std::cout << "Failed to start download" << std::endl;
			return;
		}
		shared_bytes_type bytes(mem);
		core_handle.info_map[torrent_id] = bytes; 
	}
	else if (vm.count("tor_resume")) { 
		std::cout << "Resuming..." << std::endl;
		t2h_resume_download(core_handle.handle, vm["tor_resume"].as<int>(), 1);
	}
	else if (vm.count("tor_add_tor")) { 
		std::cout << "Adding torrent.." << std::endl;
		std::string const path = vm["tor_add_tor"].as<std::string>();
		int const torrent_id = t2h_add_torrent(core_handle.handle, path.c_str());
		if (torrent_id == INVALID_TORRENT_ID) {
			std::cout << "Failed to add torrent by path : " << vm["tor_add_tor"].as<std::string>() << std::endl;
			return;
		}
		core_handle.info_map[torrent_id] = shared_bytes_type();
	}
	else if (vm.count("tor_get_info")) { 
		std::cout << "Torrent info : ";
		std::cout << t2h_get_torrent_files(core_handle.handle, vm["tor_get_info"].as<int>());
		std::cout << std::endl;
	}
	else if (vm.count("tor_get_ids")) {
		std::cout << "Avaliables ids : " << std::endl;
		std::for_each(core_handle.info_map.begin(), core_handle.info_map.end(), 
			std::cout << 
				boost::lambda::bind(&info_map_type::value_type::first, boost::lambda::_1) << 
				", ");
		std::cout << std::endl;
	}
	else if (vm.count("quit")) {
		t2h_close(core_handle.handle);
		t2h_wait(core_handle.handle);
		std::cout << "All core services stoped" << std::endl;
		exit = true;
	}
	else  {
		std::cout << "Unknown option passed" << std::endl;
		return;
	}

	std::cout << "Command execution succeed" << std::endl;
}

void dispatch_text_command(char const * ibuf, std::size_t ibuf_size, bool & exit) 
{
	namespace po = boost::program_options;
	static const char * first_value = "full_model"; 	
	po::variables_map vm;	
	std::size_t const opt_ibuff_size = 2;
	char ** opt_ibuff = new char*[opt_ibuff_size];
	po::options_description desc("Allowed options");
	
	for (std::size_t it = 0; it < opt_ibuff_size; ++it) { 
		opt_ibuff[it] = new char[256];
		std::memset(opt_ibuff, 256, 0);
	}
	std::copy(first_value, first_value + sizeof first_value / sizeof(char), opt_ibuff[0]);
	std::copy(ibuf, ibuf+ibuf_size, opt_ibuff[1]);

	desc.add_options()
			("help", "avaliable commands")
		    ("tor_stop", po::value<int>(), "take id for stop download")
			("tor_pause", po::value<int>(), "take id for pause download")
			("tor_resume", po::value<int>(), "take id for resume download")
			("tor_get_info", po::value<int>(), "take id for get torrent info")
			("tor_remove", po::value<int>(), "take id for remove torrent")
			("tor_add_tor", po::value<std::string>(), "take path to .torrent file")
			("tor_start_download", po::value<int>(), "take id for start download")
			("tor_get_ids", po::value<bool>()->implicit_value(true), "print avaliable ids")
			("quit", po::value<bool>()->implicit_value(true), "do quit")
			;
	
	po::store(po::parse_command_line(opt_ibuff_size, opt_ibuff, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) { 
		std::cout << desc << std::endl;
		return;
	}
	
	do_action_dispatch_vm(vm, exit);
	
	for (std::size_t it = 0; it < opt_ibuff_size; ++it)  
		delete [] opt_ibuff[it];
	delete [] opt_ibuff;
}

void sig_handler(int signo)
{
#if defined(UNIX) || defined(APPLE)
	if (signo != SIGINT) 
		return;
#endif
	t2h_close(core_handle.handle);
	sig_exit = true;
}

void die(std::string const & message, int error_code) 
{
	if (!message.empty())
		std::cerr << "Fatal error : " << message << std::endl;
	std::exit(error_code);
}

