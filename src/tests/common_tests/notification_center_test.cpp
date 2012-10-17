#include "notification_center.hpp"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/test/minimal.hpp>

/**
 * Helpers
 */
#define TEST_COUNTER_MAX 500	// generated events	size value
#define WAIT_TIMEOUT 5			// waitng for a good result timeout value(in ms)

// create custom event(+custom data)
#define MAKE_CUSTOM_NOTIFICATION(name, udata)											\
struct event_##name 																	\
	: public common::base_notification 													\
{																						\
    event_##name() 																		\
		: common::base_notification(__LINE__), state_(base_notification::executeable)	\
	{ }																					\
	virtual notification_state get_state() const { return state_; }						\
	virtual void set_state(notification_state state) { state_ = state; }				\
	notification_state state_;															\
	udata data;																			\
};																						

// 
#define CHECK_ENTITY(x) 									\
do {														\
	environment_init();										\
	std::cout << "Start test case " << #x << std::endl;		\
	BOOST_CHECK(x);											\
	std::cout << "End of test case" << #x << std::endl;		\
	environment_reset();									\
} while(0);				

/**
 * Test environment
 */
struct environment {
	boost::mutex lock;
	volatile bool current_test_succ;
	volatile bool on;
	volatile int executed_events;
	common::notification_center * center;
} envt;

void environment_init();
void environment_reset();
void environment_destroy(); 

static void panic(std::string const & message) 
{
	throw message;
}

/**
 * Test data
 */

MAKE_CUSTOM_NOTIFICATION(tcount, int)
typedef boost::shared_ptr<event_tcount> event_tcount_ptr; 

struct check_recv_t1 : public common::notification_receiver {
	
	check_recv_t1() : common::notification_receiver("check_recv_t1") 
	{ 
		boost::lock_guard<boost::mutex> guard(envt.lock);
		envt.current_test_succ = false;
		envt.on = true;
	}
	
	virtual void on_notify(common::notification_ptr notification)
	{ 
		if (notification && envt.on) { 
			event_tcount_ptr ev = common::notification_cast<event_tcount>(notification);
			if (ev->data == excpected_count_value) {
				boost::lock_guard<boost::mutex> guard(envt.lock);
				envt.current_test_succ = true;
				envt.on = false;
			}
		} else if (!notification) 
			panic(std::string("notification == null in ") + __FUNCTION__);
	}
	
	virtual void on_notify_failed(common::notification_ptr notification, int reason) 
	{
		boost::lock_guard<boost::mutex> guard(envt.lock);
		envt.current_test_succ = false;
		envt.on = false;	
	}

	static const int excpected_count_value = TEST_COUNTER_MAX;
};

static bool wait_for_finish(int expected_event_execution_value) 
{
	for (;;sleep(1)) {
		boost::lock_guard<boost::mutex> guard(envt.lock);
		if (!envt.on) break;
	}
	boost::lock_guard<boost::mutex> guard(envt.lock);
	return (envt.current_test_succ && expected_event_execution_value == envt.executed_events);
}

/**
 *	Test cases
 */

static inline bool check_events_recv() 
{
	bool state = false; std::size_t id = 0;
	common::notification_receiver_ptr recv(new check_recv_t1());
	boost::tie(id, state) = envt.center->add_notification_receiver(recv);
	if (!state) return false;
	
	for (int count = 0; count != TEST_COUNTER_MAX + 1; ++count) {
		event_tcount_ptr event(new event_tcount());
		event->data = count;
		envt.center->send_message(recv->get_name(), event);
	} // for
	
	state = wait_for_finish(TEST_COUNTER_MAX + 1);
	envt.center->remove_notification_receiver(recv->get_name());

	return state;
} 

static inline bool multi_threaded_check_events_recv() 
{
	std::list<boost::thread *> threads;
	bool state = false; std::size_t id = 0;
	common::notification_receiver_ptr recv(new check_recv_t1());
	boost::tie(id, state) = envt.center->add_notification_receiver(recv);
	if (!state) return false;
		
	for (int count = 0; count != TEST_COUNTER_MAX + 1; ++count) {
		event_tcount_ptr event(new event_tcount());
		event->data = count;
		threads.push_back(new 
			boost::thread(&common::notification_center::send_message, 
				envt.center, recv->get_name(), event));
		std::cout << __FUNCTION__ << std::endl; 
		if (threads.size() == 10) {
			std::cout << __FUNCTION__ << std::endl; 
			std::for_each(threads.begin(), threads.end(), 
				boost::bind(&boost::thread::join, _1));
			threads.clear();
		} // threads.size() 
	} // for
	
	std::for_each(threads.begin(), threads.end(), 
		boost::bind(&boost::thread::join, _1));

	state = wait_for_finish(TEST_COUNTER_MAX + 1);
	envt.center->remove_notification_receiver(recv->get_name());
	
	return state;
}

/**
 * Entry point
 */

int test_main(int argc, char ** argv) 
try 
{
	environment_init();
	CHECK_ENTITY(check_events_recv())
	CHECK_ENTITY(multi_threaded_check_events_recv())
	std::cout << "End ..." << std::endl;
	environment_destroy();
	return EXIT_SUCCESS;
} 
catch (std::exception const & expt) 
{
	environment_destroy();
	std::cerr << "Exception : " << expt.what() << std::endl;
	return EXIT_FAILURE;
}

/**
 * Implementations
 */

void environment_init() 
{
	boost::lock_guard<boost::mutex> guard(envt.lock);	
	if (!envt.center) {
		common::notification_center_config ncc = { 500, false };	
		envt.center = new common::notification_center(ncc);
	} 
	envt.current_test_succ = envt.on = false;
	envt.executed_events = TEST_COUNTER_MAX + 1;
}

void environment_reset() 
{
	boost::lock_guard<boost::mutex> guard(envt.lock);	
	envt.current_test_succ = envt.on = false;
	envt.executed_events = TEST_COUNTER_MAX + 1;
}

void environment_destroy() 
{
	boost::lock_guard<boost::mutex> guard(envt.lock);	
	delete envt.center; envt.center = NULL;
} 

