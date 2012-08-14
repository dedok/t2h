#ifndef ABSTRACT_SYSLOGGER_HPP_INCLUDED
#define ABSTRACT_SYSLOGGER_HPP_INCLUDED

#include <new>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * The system logger settings
 */
struct syslogger_settings {
	std::string ident;							/* logger indeficator(as example 'com.company.application') */
	std::string facility;						/* facility name(as example 'application')*/
	boost::filesystem::path log_file_path;		/* path to log file, this a optional variable(as example '/var/log/my.application.log') */
};

/**
 * Syslogger main interface
 */
class abstract_syslogger 
	: private boost::noncopyable, public boost::enable_shared_from_this<abstract_syslogger> 
{
public :
	typedef boost::shared_ptr<abstract_syslogger> ptr_type;

	explicit abstract_syslogger(syslogger_settings const & settings);
	virtual ~abstract_syslogger();

	template<class T>
	static void log_init(syslogger_settings const & settings);

	inline static abstract_syslogger::ptr_type log_instance();
	
	virtual void error(std::string const & message) = 0;
	virtual void warning(std::string const & message) = 0;
	virtual void note(std::string const & message) = 0;
	virtual void trace(std::string const & message) = 0;

private :
	static ptr_type this_instance_;

};

/** Helpers private namespace */
namespace syslogger_private {

/**
 * If current the logger is ENABLE via !SYS_LOG_OFF but not 
 * called abstract_syslogger::log_init<T>(where T implementation) or allocated of the syslogger object failed
 * the abstract_syslogger::log_instance() will return a pointer to follow(stderr_logger) object, 
 * the stderr_logger object allocated & exist at the program start and free at program end.
 */
class stderr_logger : public abstract_syslogger {
public :
	stderr_logger();
	~stderr_logger();
	
	inline static abstract_syslogger::ptr_type get(); 

	virtual void error(std::string const & message);
	virtual void warning(std::string const & message);
	virtual void note(std::string const & message);
	virtual void trace(std::string const & message);

private :
	static ptr_type std_err_instance_;

};

}

/**
 * The public static-inline function(s) 
 */
template<class T>
void abstract_syslogger::log_init(syslogger_settings const & settings) 
{
	if (abstract_syslogger::this_instance_) return;
	T * log_obj = new (std::nothrow) T(settings);
	if (log_obj) 
		abstract_syslogger::this_instance_.reset(log_obj);
}

abstract_syslogger::ptr_type abstract_syslogger::log_instance() 
{
	using namespace syslogger_private;
	abstract_syslogger::ptr_type inst = abstract_syslogger::this_instance_;
	return (!inst? stderr_logger::get() : inst);
}

/**
 * The private static-inline function(s)
 */
abstract_syslogger::ptr_type syslogger_private::stderr_logger::get() 
{
	return std_err_instance_;
}

#endif

