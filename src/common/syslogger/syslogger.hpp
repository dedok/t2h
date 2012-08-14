#ifndef SYSLOGGER_HPP_INCLUDED
#define SYSLOGGER_HPP_INCLUDED

#include "abstract_syslogger.hpp"
#include "syslogger_impl.hpp"

#include <cstdio>
#include <cstring>

#define SYS_LOGGER_MAX_MESSAGE_SIZE 256 * sizeof(char)

#define TEMPLATE_SYS_LOG_NOTIFY_(log_type, ...)					\
do {															\
	char message_[SYS_LOGGER_MAX_MESSAGE_SIZE + 1];				\
	std::memset(message_, '\0', SYS_LOGGER_MAX_MESSAGE_SIZE);	\
	std::sprintf(message_, __VA_ARGS__);						\
	log_type(message_);											\
} while(0); 

#define DEBUG
#if !defined(SYS_LOGGER_OFF) 

#	define LOG_INIT(settings) \
		abstract_syslogger::log_init<syslogger_impl>(settings);

#	define LOG_WARNING(...) \
		TEMPLATE_SYS_LOG_NOTIFY_(abstract_syslogger::log_instance()->warning, __VA_ARGS__)

#	define LOG_ERROR(...) \
		TEMPLATE_SYS_LOG_NOTIFY_(abstract_syslogger::log_instance()->error, __VA_ARGS__)

#	if defined(DEBUG)

#		define LOG_TRACE(...) \
			TEMPLATE_SYS_LOG_NOTIFY_(abstract_syslogger::log_instance()->trace, __VA_ARGS__)

#		define LOG_NOTICE(...) \
			TEMPLATE_SYS_LOG_NOTIFY_(abstract_syslogger::log_instance()->note, __VA_ARGS__)

#	else // !DEBUG
#		define LOG_TRACE(...) do { } while(0);
#		define LOG_NOTICE(...) do { } while(0);
#	endif 
#else // SYS_LOGGER_OFF
#	define LOG_INIT(settings) do { } while(0);
#	define LOG_WARNING(...) do { } while(0);
#	define LOG_ERROR(...) do { } while(0);
#	define LOG_TRACE(...) do { } while(0);
#	define LOG_NOTICE(...) do { } while(0);
#endif

#endif

