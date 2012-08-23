#ifndef BASE_SERVICE_HPP_INCLUDED
#define BASE_SERVICE_HPP_INCLUDED

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace common { 

class base_service : private boost::noncopyable {
public :	
	typedef boost::shared_ptr<base_service> ptr_type;

	enum service_state {
		service_running = 0x1,
		service_stoped,
		service_state_unknown = service_stoped + 0x1
	};

	explicit base_service(std::string const & name);
	virtual ~base_service();

	virtual bool launch_service() = 0;
	virtual void stop_service() = 0;
	virtual void wait_service() = 0;
	
	virtual ptr_type clone() = 0;
	
	virtual service_state get_service_state() const = 0;
	
	std::string service_name() const;

private :
	std::string const service_name_;

};

typedef	base_service::ptr_type base_service_ptr;

} // namespace common

#endif

