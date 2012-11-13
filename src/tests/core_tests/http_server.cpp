#include "http_server_core.hpp"

#include "http_utility.hpp"
#include "hc_event_source_adapter.hpp"
#include "core_notification_center.hpp"

#include <list>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

/**
 * Definitions and const|&static data
 */

#define DEBUG
#include "syslogger.hpp"

#if defined(__APPLE__)
#include <signal.h>
#endif

#include <iostream>

common::base_service_ptr http_server;
t2h_core::setting_manager_ptr setting_manager = t2h_core::setting_manager::shared_manager();

static syslogger_settings const log_settings = {
	"com.t2h.HttpServer", 
	"application",
	"application.log"
};

struct program_options {
	int update_state;
	std::string config_path;
};

/**
 * Helpers
 */

inline static void die(std::string const & message, int exit_code) 
{
	std::cerr << message << std::endl;
	std::exit(exit_code);
}

static void sig_handler(int signo)
{
#if defined(__APPLE__)
	if (signo != SIGINT) 
		return;
#else
	std::cin.get();
#endif // _APPLE_
	if (http_server) { 
		http_server->stop_service();
		http_server->wait_service();
	}
	die("Http server quit.", 0);
}


static inline program_options get_options(int argc, char ** argv) 
{
	namespace po = boost::program_options;
	program_options options;
	
	po::variables_map vm;	
	po::options_description desc("Allowed options");
	
	try 
	{
		desc.add_options()
		    ("help", "produce help message")
			    ("with-config", po::value<std::string>(&options.config_path), "config path")
				("update-state", po::value<int>(&options.update_state), "update state 1 - slow, 2 - fast, other numbers - default")
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

/**
 * core_notification_center
 */
class core_notification_source {
public :
	core_notification_source() { t2h_core::core_notification_center(); }
	~core_notification_source() { }

	void initialize() 
	{
		boost::system::error_code ec;
		std::string const doc_root = setting_manager->get_value<std::string>("doc_root");
		for (boost::filesystem::recursive_directory_iterator end, 
				dir(doc_root); 
			dir != end; 
			++dir) 
		{
			if (boost::filesystem::is_directory(*dir, ec))
				continue;	
			std::string const path = utility::http_normalize_uri(dir->path().string());
			file_data fd = { path, boost::filesystem::file_size(*dir, ec) };
			if (!ec) {
				files_in_root_.push_back(fd);
				std::cout << "traked file : " << fd.path << std::endl;
				event_sender_.on_file_add(fd.path, fd.size);
			} // if
		} // for
	}
	
	void slow_files_info_update() 
	{
		Sleep(15000);
		for (std::list<file_data>::iterator first = files_in_root_.begin(), last = files_in_root_.end();
			first != last;
			++first)
		{
			boost::int64_t const offset = first->size / 30;
			boost::int64_t cur_offset = offset;
			while (cur_offset < first->size) {	
#if defined(WIN32)
				Sleep(1000);
#else
				sleep(1);
#endif // WIN32
				cur_offset += offset;
				if (cur_offset > first->size)
					cur_offset = first->size;
				event_sender_.on_progress_update(first->path, cur_offset);
			} // while
			event_sender_.on_file_complete(first->path, first->size);
		} // for	
	}

	void default_files_info_update() 
	{
		for (std::list<file_data>::iterator first = files_in_root_.begin(), last = files_in_root_.end();
			first != last;
			++first)
		{
			event_sender_.on_file_complete(first->path, first->size);
		} // for	
	}

	void fast_files_info_update() 
	{
		for (std::list<file_data>::iterator first = files_in_root_.begin(), last = files_in_root_.end();
			first != last;
			++first)
		{
			boost::int64_t const offset = first->size / 4;
			boost::int64_t cur_offset = offset;
			while (cur_offset < first->size) {	
				cur_offset += offset;
				if (cur_offset > first->size)
					cur_offset = first->size;
				event_sender_.on_progress_update(first->path, cur_offset);
			} // while
			event_sender_.on_file_complete(first->path, first->size);
		} // for	
	}

private :
	struct file_data {
		std::string path;
		boost::int64_t size;
	};

	std::list<file_data> files_in_root_; 
	t2h_core::details::hc_event_source_adapter event_sender_;
};

/**
 *	Entry point
 */
int main(int argc, char* argv[])
{

#if defined(__APPLE__)
	if (signal(SIGINT, sig_handler) == SIG_ERR) 
	 	die("failed to add signal handler", 1);
#endif // __APPLE__
	program_options po = get_options(argc, argv);
	
	LOG_INIT(log_settings)

	try 
	{
		common::notification_center_ptr nc = t2h_core::core_notification_center();
		core_notification_source cns;
		setting_manager->load_config(po.config_path);
		http_server.reset(new t2h_core::http_server_core(setting_manager));
		
		if (!http_server->launch_service())
			die("launch failed", -1);

		cns.initialize();	
		switch (po.update_state) 
		{
			case 1 :
				std::cout << "Slow files update enable." << std::endl;
				cns.slow_files_info_update();
			break;
			case 2 : 
				std::cout << "Fast files update enable." << std::endl;
				cns.fast_files_info_update();
			default :
				std::cout << "Default files update enable." << std::endl;
				cns.default_files_info_update();
			break;
		}
#if defined(WIN32)
		sig_handler(0);
#else
		http_server->wait_service();
#endif //WIN32
	} 
	catch (std::exception const & expt) 
	{
		die(expt.what(), -2);
	}

	return 0;
}

