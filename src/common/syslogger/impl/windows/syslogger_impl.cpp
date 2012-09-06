#include "syslogger_impl.hpp"

/**
 * syslogger_impl helpers
 */
 namespace details {
		
} // namespace details
#define MAX_STRINGS_PER_REPORT 1
#define REPORT_EVENT_GENERIC(handle, type, message)	\
	if (handle != INVALID_HANDLE_VALUE)	{			\
		char const * message_ = message.c_str();	\
		ReportEventA(handle,						\
					type, 							\
					0, 								\
					0, 								\
					NULL,							\
					MAX_STRINGS_PER_REPORT, 		\
					0, 								\
					(LPCSTR *)&message_, 			\
					NULL); 							\
	}											

/**
 * Public syslogger_impl api
 */
syslogger_impl::syslogger_impl(syslogger_settings const & settings) 
	: abstract_syslogger(settings), settings_(settings), log_handle_(INVALID_HANDLE_VALUE)
{
	init_event_source();
}

syslogger_impl::~syslogger_impl() 
{
	destroy_event_source();
}

void syslogger_impl::error(std::string const & message) 
{
	REPORT_EVENT_GENERIC(log_handle_, 
		EVENTLOG_ERROR_TYPE, message)
}

void syslogger_impl::warning(std::string const & message) 
{
	REPORT_EVENT_GENERIC(log_handle_, 
		EVENTLOG_WARNING_TYPE, message)
}

void syslogger_impl::note(std::string const & message) 
{
	REPORT_EVENT_GENERIC(log_handle_, 
		EVENTLOG_INFORMATION_TYPE, message)
}

void syslogger_impl::trace(std::string const & message) 
{
	note(message);
}

/**
 * Private syslogger_impl api
 */
void syslogger_impl::init_event_source() 
{
	std::string const event_source_name = settings_.ident + "_" + settings_.facility;
	log_handle_ = RegisterEventSource(NULL, (LPCSTR)event_source_name.c_str());
	if (GetLastError() == ERROR_ACCESS_DENIED)
		log_handle_ = INVALID_HANDLE_VALUE;
}

void syslogger_impl::destroy_event_source() 
{
	if (log_handle_ != INVALID_HANDLE_VALUE) {
		DeregisterEventSource(log_handle_);
		log_handle_ = INVALID_HANDLE_VALUE;
	}
}
