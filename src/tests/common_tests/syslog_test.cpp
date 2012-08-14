#define DEBUG
#include "syslogger.hpp"

#include <vector>
#include <boost/thread.hpp>

struct counter { 
	int c;
	boost::mutex lock;
};

static syslogger_settings const log_settings = { 
	"com.company.application", 
	"application", 
	"/Users/dedokOne/application.log" 
};

static counter count;

static void log_it() 
{
	int count_value = 0;
	{ // lock follow scope
		boost::lock_guard<boost::mutex> lock(count.lock);
		count_value = count.c;
		++count.c;
	} // unlock
	LOG_WARNING("int %i message, with string %s", count_value, "text")
	LOG_TRACE("int %i message, with string %s", count_value, "text")
	LOG_NOTICE("int %i message, with string %s", count_value, "text")
	LOG_ERROR("int %i message, with string %s", count_value, "text")
}

int main(int argc, char ** argv) 
{
	
	LOG_INIT(log_settings)
	log_it();

	std::size_t const threads_size = 25;
	std::vector<boost::thread *> threads; threads.resize(threads_size);
	for (std::vector<boost::thread *>::iterator it = threads.begin(); 
		it != threads.end(); 
		++it) 
	{
		(*it) = new boost::thread(log_it);
	} 
	
	for (std::vector<boost::thread *>::iterator it = threads.begin(); 
		it != threads.end(); 
		++it) 
	{
		(*it)->join();
	} 
	
	return 0;
} 

