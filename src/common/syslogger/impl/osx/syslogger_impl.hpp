#ifndef SYSLOGGER_IMPL_HPP_INCLUDED
#define SYSLOGGER_IMPL_HPP_INCLUDED

#include "abstract_syslogger.hpp"

#include <asl.h>
#include <fcntl.h>
#include <unistd.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/signals2.hpp>

class syslogger_impl : public abstract_syslogger { 
public :
	explicit syslogger_impl(syslogger_settings const & settings);
	~syslogger_impl();

	virtual void error(std::string const & message);
	virtual void warning(std::string const & message);
	virtual void note(std::string const & message);
	virtual void trace(std::string const & message);

private :
	typedef boost::signals2::signal<void(std::string const & message)> signal_type;
	static const std::size_t signals_size = 4;
	
	void init_logger();
	void close_logger();
	
	void connect_log_event_signals();
	void disconnect_log_event_signals();

	void error_(std::string const & message);
	void warning_(std::string const & message);
	void note_(std::string const & message);
	void trace_(std::string const & message);

	syslogger_settings settings_;
	boost::mutex lock_;
	struct {	
		int fd;
		aslclient handle;	
		signal_type signals[signals_size];
	} log_;
};

#endif

