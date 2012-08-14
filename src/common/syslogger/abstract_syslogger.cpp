#include "abstract_syslogger.hpp"

#include <iostream>

/**
 * syslogger main interface
 */
abstract_syslogger::abstract_syslogger(syslogger_settings const & settings) 
{
}

abstract_syslogger::~abstract_syslogger() 
{
}

abstract_syslogger::ptr_type abstract_syslogger::this_instance_;

/**
 * syslogger private helper(s)
 */
namespace syslogger_private {

stderr_logger::stderr_logger() : abstract_syslogger(syslogger_settings()) 
{ 
}
	
stderr_logger::~stderr_logger() 
{
}
	
void stderr_logger::error(std::string const & message) 
{
	std::cerr << message << std::endl;
}

void stderr_logger::warning(std::string const & message) 
{
	std::cerr << message << std::endl;
}

void stderr_logger::note(std::string const & message) 
{
	std::cerr << message << std::endl;
}

void stderr_logger::trace(std::string const & message) 
{
	std::cerr << message << std::endl;
}

abstract_syslogger::ptr_type stderr_logger::std_err_instance_(new (std::nothrow) stderr_logger());

} // !syslogger_private

