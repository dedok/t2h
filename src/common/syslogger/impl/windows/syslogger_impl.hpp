#ifndef SYSLOGGER_IMPL_HPP_INCLUDED
#define SYSLOGGER_IMPL_HPP_INCLUDED

#include "abstract_syslogger.hpp"

class syslogger_impl : public abstract_syslogger { 
public :
	explicit syslogger_impl(syslogger_settings const & settings);
	~syslogger_impl();

	virtual void error(std::string const & message);
	virtual void warning(std::string const & message);
	virtual void note(std::string const & message);
	virtual void trace(std::string const & message);

private :
	syslogger_settings settings_;

};

#endif

