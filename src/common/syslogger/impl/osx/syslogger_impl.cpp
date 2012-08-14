#include "syslogger_impl.hpp"

/**
 * syslogger_impl private helpers
 */
#define EXCEPTION_SAFE(expression)\
do { \
	try { \
		expression; \
	} catch (...) { /* do nothing */ } \
} while(0);

namespace syslogger_impl_private {
	static const std::size_t ERROR = 0x0;
	static const std::size_t WARNING = 0x1;
	static const std::size_t NOTE = 0x2;
	static const std::size_t TRACE = 0x3;
}

/**
 * syslogger_impl public api
 */
syslogger_impl::syslogger_impl(syslogger_settings const & settings) 
	: abstract_syslogger(settings), settings_(settings), log_() 
{
	init_logger();
}

syslogger_impl::~syslogger_impl() 
{
	close_logger();
}

void syslogger_impl::error(std::string const & message) 
{
	EXCEPTION_SAFE(log_.signals[syslogger_impl_private::ERROR](message))
}

void syslogger_impl::warning(std::string const & message) 
{
	EXCEPTION_SAFE(log_.signals[syslogger_impl_private::WARNING](message))
}

void syslogger_impl::note(std::string const & message) 
{
	EXCEPTION_SAFE(log_.signals[syslogger_impl_private::NOTE](message))
}

void syslogger_impl::trace(std::string const & message) 
{
	EXCEPTION_SAFE(log_.signals[syslogger_impl_private::TRACE](message))
}

/**
 * syslogger_impl private api
 */
void syslogger_impl::init_logger() 
{
	int const asl_open_opt = ASL_OPT_STDERR | ASL_OPT_NO_DELAY;
	if (!log_.handle) {
		if ((log_.fd = open(settings_.log_file_path.string().c_str(), 
			O_RDWR | O_CREAT)) < 0) 
		{	
			return;
		}
		log_.handle = asl_open(settings_.ident.c_str(), 
								settings_.facility.c_str(), 
								asl_open_opt);
		if (!log_.handle && 
			asl_add_log_file(log_.handle, log_.fd) > 0) 
		{ 
			close_logger();
			return;
		}
		connect_log_event_signals();	
	}
}

void syslogger_impl::close_logger() 
{		
	for (std::size_t it = 0; it < syslogger_impl::signals_size; ++it)
		log_.signals[it].disconnect_all_slots();

	if (log_.handle) {
		asl_close(log_.handle); 
		log_.handle = NULL;
	}

	if (log_.fd > 0) {
		close(log_.fd);	
		log_.fd = -1;	
	}
}

void syslogger_impl::connect_log_event_signals() 
{
	using namespace syslogger_impl_private;
	log_.signals[TRACE].connect(boost::bind(&syslogger_impl::trace_, this, _1));
	log_.signals[NOTE].connect(boost::bind(&syslogger_impl::note_, this, _1));
	log_.signals[ERROR].connect(boost::bind(&syslogger_impl::error_, this, _1));
	log_.signals[WARNING].connect(boost::bind(&syslogger_impl::warning_, this, _1));
}

void syslogger_impl::error_(std::string const & message) 
{
	if (!log_.handle) return;
	asl_log(log_.handle, NULL, ASL_LEVEL_EMERG, "%s, errno code '%i', message '%s'", 
		message.c_str(), errno, strerror(errno));
}

void syslogger_impl::warning_(std::string const & message) 
{
	if (!log_.handle) return;
	asl_log(log_.handle, NULL, ASL_LEVEL_ERR, "%s, errno code '%i', message '%s'", 
		message.c_str(), errno, strerror(errno));
}

void syslogger_impl::note_(std::string const & message) 
{
	if (!log_.handle) return;
	asl_log(log_.handle, NULL, ASL_LEVEL_INFO, "%s, errno code '%i', message '%s'", 
		message.c_str(), errno, strerror(errno));
}

void syslogger_impl::trace_(std::string const & message) 
{
	if (!log_.handle) return;
	asl_log(log_.handle, NULL, ASL_LEVEL_NOTICE, "%s, errno code '%i', message '%s'", 
		message.c_str(), errno, strerror(errno));
}

