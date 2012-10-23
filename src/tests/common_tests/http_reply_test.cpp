#include "http_utility.hpp"

#include <boost/bind.hpp>
#include <boost/test/minimal.hpp>

#define COMBINE_STOCK_REPS(type, b) 															\
do {																							\
	fill_stock_reply(type);																		\
	std::for_each(buffer.begin(), buffer.end(),													\
			boost::bind(&utility::http_reply::buffer_type::push_back, boost::ref(b), _1));		\
} while(0);																	

#define CHECK_WITH_CLEAN(x) { reset_envt(); BOOST_CHECK(x); }

namespace {

/**
 * Test date
 */
utility::http_reply::buffer_type buffer;
utility::http_reply reply(buffer);

/**
 * Helpers
 */

static inline void reset_envt() 
	{ buffer.clear(); }

static inline void fill_stock_reply(utility::http_reply::status_type reply_type) 
{	
	using namespace utility;
	buffer.clear();
	reply.stock_reply(reply_type);
	std::copy(buffer.begin(), buffer.end(),
           std::ostream_iterator<char>(std::cout));
}

/**
 * Test cases
 */

static bool add_test()
{
	using namespace utility;

	std::string const expected_result = "HTTP/1.1 200 OK\r\n"
										"C1: S\r\n"
										"C2: S\r\n"
										"C3: S\r\n"
										"\r\n"
										"the_content"; 
	
	std::string const test_content = "the_content";
	std::string::size_type const tc_size = test_content.size();
	reply.add_status(http_reply::ok);
	reply.add_header("C1", "S"); reply.add_header("C2", "S"); reply.add_header("C3", "S");
	if (!reply.add_content(test_content.c_str(), tc_size))
		return false;
	return std::equal(buffer.begin(), buffer.end(), expected_result.begin());
}

static bool fail_add_test() 
{
	using namespace utility;
	
	bool add_result = false;
	try 
	{
		reply.enable_buf_realocation(false);
		buffer.resize(10);
		add_result = add_test();
	}
	catch (std::exception const & expt) 
	{
		return true;
	}
	return !add_result;
}

static bool nonreadlocated_add_test() 
{
	// TODO investigate this case
	using namespace utility;

	bool add_result = false;	
	std::size_t buf_fixed_size = 51;
	try 
	{
		reply.enable_buf_realocation(false);
		buffer.resize(buf_fixed_size);
		add_result = add_test();
	}
	catch (std::exception const & expt)
	{
		return false;
	}
	return add_result;
}

static inline bool stock_reply_test() 
{
	using namespace utility;
	
	http_reply::buffer_type b_;
	
	COMBINE_STOCK_REPS(http_reply::ok, b_);
	COMBINE_STOCK_REPS(http_reply::created, b_);
	COMBINE_STOCK_REPS(http_reply::accepted, b_);
	COMBINE_STOCK_REPS(http_reply::no_content, b_);
	COMBINE_STOCK_REPS(http_reply::partial_content, b_);
	COMBINE_STOCK_REPS(http_reply::multiple_choices, b_);
	COMBINE_STOCK_REPS(http_reply::moved_temporarily, b_);
	COMBINE_STOCK_REPS(http_reply::moved_permanently, b_);
	COMBINE_STOCK_REPS(http_reply::not_modified, b_);
	COMBINE_STOCK_REPS(http_reply::bad_request, b_);
	COMBINE_STOCK_REPS(http_reply::unauthorized, b_);
	COMBINE_STOCK_REPS(http_reply::forbidden, b_);
	COMBINE_STOCK_REPS(http_reply::not_found, b_);
	COMBINE_STOCK_REPS(http_reply::internal_server_error, b_);
	COMBINE_STOCK_REPS(http_reply::not_implemented, b_);
	COMBINE_STOCK_REPS(http_reply::bad_gateway, b_);
	COMBINE_STOCK_REPS(http_reply::service_unavailable, b_);
	
//	if (expected_stock_reps.size() != b_.size())
//		return false;

//	for (http_reply::buffer_type::const_iterator first = 
//			b_.begin(), last = b_.end(), esr_it = expected_stock_reps.begin();
//		first != last; 
//		++first, ++esr_it) 
//	{
//		if (*first != *esr_it)
//			return false;
//	}
	return true;
}

} // namesapce

int test_main(int argc, char ** argv) 
{
	CHECK_WITH_CLEAN(add_test());
	CHECK_WITH_CLEAN(fail_add_test());
//	CHECK_WITH_CLEAN(nonreadlocated_add_test());
// TODO implement follow test 
	CHECK_WITH_CLEAN(stock_reply_test());
	return 0;
}

