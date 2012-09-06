#include "syslogger_impl.hpp"

/**
 *
 */
syslogger_impl::syslogger_impl(syslogger_settings const & settings) 
	: abstract_syslogger(settings), settings_(settings) 
{
}

syslogger_impl::~syslogger_impl() 
{
}

void syslogger_impl::error(std::string const & message) 
{
}

void syslogger_impl::warning(std::string const & message) 
{
}

void syslogger_impl::note(std::string const & message) 
{
}

void syslogger_impl::trace(std::string const & message) 
{
}

