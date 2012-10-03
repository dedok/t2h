#if defined(T2H_CORE_SHARED)
#	define T2H_IMPORT
#endif // T2H_CORE_SHARED

#include "t2h.h"

#if defined(UNIX) || defined(__APPLE__)
#	include <signal.h>
#elif defined(WIN32)
#	if !defined(WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN  
#	endif
#	include <windows.h>
#endif

#include <map>
#include <string>
#include <locale>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_array.hpp>
#include <boost/program_options.hpp>

/** 
 * Helpers declaraion 
 */
#if defined(__APPLE__) || defined(UNIX)
void sig_handler(int signo);
#elif defined(WIN32)
BOOL __stdcall console_handler(DWORD reason_type);
#endif

void die(std::string const & message, int error_code = 1); 

/** 
 * Type declarations 
 */
typedef boost::shared_array<char> shared_bytes_type;
typedef std::map<T2H_SIZE_TYPE, shared_bytes_type> info_map_type;

struct t2h_handle_wrapper {
	t2h_handle_t handle;
	boost::mutex im_lock;
	info_map_type info_map;
};

static t2h_handle_wrapper core_handle;

struct signal_handle_state{
	bool sig_exit;
	boost::mutex lock;
}; 

static signal_handle_state sig_state;

struct program_options{
	bool is_valid;
	std::string config_path;
	std::string torrent_dir;
};

/** 
 * Main functionality 
 */
void console_controller(); 

void addrents_from_directory(t2h_handle_wrapper & handle, program_options const & options); 

program_options get_options(int argc, char ** argv); 

void _dispatch_vm(boost::program_options::variables_map const & vm, bool & exit); 

void dispatch_text_command(char const * ibuf, std::size_t ibuf_size, bool & exit); 

/** 
 * Entry point 
 */
int main(int argc, char ** argv) 
{
	sig_state.sig_exit = false;

	try 
	{
#if defined(UNIX) || defined(__APPLE__)
		if (signal(SIGINT, sig_handler) == SIG_ERR) 
		 	die("failed to add signal handler", 1);
#elif defined(WIN32)
		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)console_handler, TRUE) == FALSE)
			die("failed to add console handler", 1);
#endif
		program_options options = get_options(argc, argv);
	
		core_handle.handle = t2h_init(options.config_path.c_str());
		if (!core_handle.handle)
			die("failed to start torrent to http core", 2);

		addrents_from_directory(core_handle, options);

		std::cout << "t2h[" << T2H_VERSION_STRING << "] run..." << std::endl;
		boost::thread console_controller_loop(&console_controller);		
		console_controller_loop.join();
	} 
	catch (std::exception const & expt) 
	{ 
		die(expt.what(), 3);
	}

	std::cout << "Exiting..." << std::endl;
	t2h_close(core_handle.handle);
	std::cout << "All core services stoped" << std::endl;

	return 0;
}

/** 
 * Helpers implementation 
 */

static void get_input_stream(char * buffer, std::size_t buffer_size) 
{
	static std::size_t const enter_key_code = (std::size_t)'\n';

	std::fill(buffer, buffer+buffer_size, '\0');
	
	std::size_t pos = 0; 
	char current_symbol = '\0';
	while (std::cin.good() && pos < buffer_size) {
		if (std::size_t(current_symbol = std::cin.get()) == enter_key_code) 
			break;
		buffer[pos] = current_symbol, ++pos; 
	}
}

void console_controller() 
{
	std::size_t const ibuf_size = 256;
	char ibuf[ibuf_size];
	for (bool exit_state = false;;) 
	{
		std::cout << "Waiting next command..." << std::endl;
		try 
		{	
			{ // sig_state lock zone
			boost::lock_guard<boost::mutex> guard(sig_state.lock);
			if (exit_state || sig_state.sig_exit) break;
			} // sig_state lock zone end
			
			get_input_stream(ibuf, ibuf_size);
			
			dispatch_text_command(ibuf, ibuf_size, exit_state);	
		} 
		catch (std::exception const & expt) 
		{
			std::cout << "Command error : " << expt.what() << std::endl;
		}
	} // !loop	
}

void addrents_from_directory(t2h_handle_wrapper & handle, program_options const & options) 
{
	using namespace boost::filesystem;
	
	boost::system::error_code error_code;
	if (!exists(options.torrent_dir, error_code))
		die(std::string("torrent directory not exist " + options.torrent_dir), 1);
	
	std::cout << "Read torrents directory : " << options.torrent_dir << std::endl;
	directory_iterator it(options.torrent_dir), end;
	for (;it != end; ++it) {
		T2H_SIZE_TYPE torrent_id = t2h_add_torrent(handle.handle, it->path().string().c_str());
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

template<typename T, typename P>
static T my_remove_if(T beg, T end, P pred)
{
	T dest = beg;
	for (T itr = beg;itr != end; ++itr)
		if (!pred(*itr))
			*(dest++) = *itr;
	return dest;
}

static inline bool is_space_(char c) 
{	
	std::locale loc;
	return std::isspace(c, loc);
}

static inline boost::tuple<bool, T2H_SIZE_TYPE, T2H_SIZE_TYPE> get_from_string_(std::string const & str) 
{
	bool state = false;
	T2H_SIZE_TYPE torrent_id = 0, file_id = 0;
	try 
	{
		std::string cleared_string = str;
		cleared_string.erase(my_remove_if(cleared_string.begin(), cleared_string.end(), 
			is_space_), 
			cleared_string.end());
		std::size_t const pos = str.find_first_of(",");
		torrent_id = boost::lexical_cast<T2H_SIZE_TYPE>(str.substr(0, pos)); 
		file_id = boost::lexical_cast<T2H_SIZE_TYPE>(str.substr(pos + 1, str.size()));
		state = true;
	}
	catch (std::exception const & expt) 
	{ /* do nothing */ }
	return boost::make_tuple(state, torrent_id, file_id);
}

void _dispatch_vm(boost::program_options::variables_map const & vm, bool & exit) 
{
	boost::lock_guard<boost::mutex> guard(core_handle.im_lock);
	if (vm.count("stop")) { 
		std::cout << "Stoping..." << std::endl;
		t2h_stop_download(core_handle.handle, vm["stop"].as<T2H_SIZE_TYPE>());
	}
	else if (vm.count("pause")) { 
		std::cout << "Pausing..." << std::endl;
		bool state = false;
		std::string const string_ids = vm["pause"].as<std::string>();
		std::pair<T2H_SIZE_TYPE, T2H_SIZE_TYPE> ids_pair;
		boost::tie(state, ids_pair.first, ids_pair.second) = get_from_string_(string_ids);
		if (!state) { 
			std::cout << "Failed to pause download : not valid args passed (" << string_ids << ")" << std::endl;
			return;
		}
		t2h_paused_download(core_handle.handle, ids_pair.first, ids_pair.second);	
	}
	else if (vm.count("remove")) { 
		std::cout << "Removing..." << std::endl;
		t2h_delete_torrent(core_handle.handle, vm["remove"].as<T2H_SIZE_TYPE>());
	}
	else if (vm.count("start_download")) {	
		std::cout << "Staring download..." << std::endl;	
		bool state = false;
		std::string const string_ids = vm["start_download"].as<std::string>();
		std::pair<T2H_SIZE_TYPE, T2H_SIZE_TYPE> ids_pair;
		boost::tie(state, ids_pair.first, ids_pair.second) = get_from_string_(string_ids);
		if (!state) { 
			std::cout << "Failed to start download : not valid args passed (" << string_ids << ")" << std::endl;
			return;
		}
		if (!core_handle.info_map.count(ids_pair.first)) {
			std::cout << "Failed to start download : no such torrent id ( ="  << ids_pair.first << ")" << std::endl;
			return;
		}
		char * mem = t2h_start_download(core_handle.handle, ids_pair.first, ids_pair.second);
		if (!mem) {
			std::cout << "Failed to start download, file no not exists" << std::endl;
			return;
		}
		shared_bytes_type bytes(mem);
		std::cout << "Torrent url for download : " << mem << std::endl;
	}
	else if (vm.count("resume")) { 
		std::cout << "Resuming..." << std::endl;
		t2h_resume_download(core_handle.handle, vm["resume"].as<T2H_SIZE_TYPE>(), 1);
	}
	else if (vm.count("add")) { 
		std::cout << "Adding torrent.." << std::endl;
		std::string const path = vm["add"].as<std::string>();
		T2H_SIZE_TYPE const torrent_id = t2h_add_torrent(core_handle.handle, path.c_str());
		if (torrent_id == INVALID_TORRENT_ID) {
			std::cout << "Failed to add torrent by path : " << vm["add"].as<std::string>() << std::endl;
			return;
		}
		core_handle.info_map[torrent_id] = shared_bytes_type();
	}
	else if (vm.count("get_info")) { 
		std::cout << "Torrent info : ";
		std::cout << t2h_get_torrent_files(core_handle.handle, vm["get_info"].as<T2H_SIZE_TYPE>());
		std::cout << std::endl;
	}
	else if (vm.count("get_ids")) {
		std::cout << "Avaliables ids : " << std::endl;
		std::for_each(core_handle.info_map.begin(), core_handle.info_map.end(), 
			std::cout << 
				boost::lambda::bind(&info_map_type::value_type::first, boost::lambda::_1) << 
				", ");
		std::cout << std::endl;
	}
	else if (vm.count("quit")) {
		exit = true;
	}
	else  {
		std::cout << "Unknown command passed" << std::endl;
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
		    ("stop", po::value<T2H_SIZE_TYPE>(), "take id for stop download")
			("pause", po::value<T2H_SIZE_TYPE>(), "take id for pause download")
			("resume", po::value<T2H_SIZE_TYPE>(), "take id for resume download")
			("get_info", po::value<T2H_SIZE_TYPE>(), "take id for get torrent info")
			("remove", po::value<T2H_SIZE_TYPE>(), "take id for remove torrent")
			("add", po::value<std::string>(), "take path to .torrent file")
			("start_download", po::value<std::string>(), 
					"take first torrent id for start download, socond file id")
			("get_ids", po::value<bool>()->implicit_value(true), "print avaliable ids")
			("quit", po::value<bool>()->implicit_value(true), "do quit")
			;
	
	po::store(po::parse_command_line(opt_ibuff_size, opt_ibuff, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) { 
		std::cout << desc << std::endl;
		return;
	}
	
	_dispatch_vm(vm, exit);
	
	for (std::size_t it = 0; it < opt_ibuff_size; ++it)  
		delete [] opt_ibuff[it];
	delete [] opt_ibuff;
}

#if defined(UNIX) || defined(__APPLE__)
void sig_handler(int signo)
{
	if (signo != SIGINT) 
		return;
	boost::lock_guard<boost::mutex> guard(sig_state.lock);
	sig_state.sig_exit = true;
	std::cout << "Press 'enter' for exit..." << std::endl;
}
#elif defined(WIN32)
BOOL __stdcall console_handler(DWORD reason_type) 
{
	switch (reason_type) 
	{
		case CTRL_C_EVENT: case CTRL_BREAK_EVENT: case CTRL_CLOSE_EVENT: 
		case CTRL_SHUTDOWN_EVENT: case CTRL_LOGOFF_EVENT: 
			sig_state.lock.lock();
			sig_state.sig_exit = true;
			std::cout << "Press 'enter' for exit..." << std::endl;
			sig_state.lock.unlock();
		default : break;
	} 
	return TRUE;
}
#endif

void die(std::string const & message, int error_code) 
{
	if (!message.empty())
		std::cerr << "Fatal error : " << message << std::endl;
	std::exit(error_code);
}

