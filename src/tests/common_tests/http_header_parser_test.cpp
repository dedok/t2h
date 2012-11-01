#include "http_utility.hpp"

#include <iostream>
#include <boost/test/minimal.hpp>

#define FIRST_TEST_CASE(value, f, l, x, x1) 		\
do {												\
	init_header_list_with_header(value);			\
	BOOST_CHECK(x(f, l) == x1);						\
	reset_header_list();							\
} while(0);

#define SECOND_TEST_CASE(value, f, l, f1, l1, x, x1) 		\
do {														\
	init_header_list_with_header(value);					\
	BOOST_CHECK(x(f, l, f1, l1) == x1);						\
	reset_header_list();									\
} while(0);

#define THIRD_TEST_CASE(value, x) 							\
do {														\
	init_header_list_with_header(value);					\
	BOOST_CHECK(x() == true);								\
	reset_header_list();									\
} while(0);


namespace {

utility::header_list_type headers; 

static inline void init_header_list_with_header(std::string const & value) 
{
	using namespace utility;
	http_header header = { "Range", value };
	headers.push_back(header);
}

static inline void reset_header_list() 
	{ headers.clear(); }

static inline bool test_1(boost::int64_t expected_begin, boost::int64_t expected_end) 
{
	using namespace utility;
	range_header rheader;
	if (!http_translate_range_header(rheader, headers))
		return false;	
	return (rheader.bstart_1 == expected_begin && rheader.bend_1 == expected_end);
}

static inline bool test_2(boost::int64_t eb_1, boost::int64_t ee_1, boost::int64_t eb_2, boost::int64_t ee_2) 
{
	using namespace utility;
	range_header rheader;
	if (!http_translate_range_header(rheader, headers))
		return false;	
	return (rheader.bstart_1 == eb_1 && rheader.bend_1 == ee_1 &&
			rheader.bstart_2 == eb_2 && rheader.bend_2 == ee_2);
}

static inline bool test_3() 
{
	using namespace utility;
	range_header rheader;
	return http_translate_range_header(rheader, headers);
}

} // namespace

struct helper_1 {
	char const * range_header;
	boost::int64_t ex_begin;
	boost::int64_t ex_end;
	bool ex_result;
} test_data_case_1[] = { 
	{"bytes=-1", utility::range_header::all, 1, true},  
	{"bytes=1-", 1, utility::range_header::all, true},  
	{"bytes=-", utility::range_header::all, utility::range_header::all, true},  
	{"bytes=*-1", utility::range_header::all, 1, true},  
	{"bytes=1-*", 1, utility::range_header::all, true},  
	{"bytes=*-*", utility::range_header::all, utility::range_header::all, true},  
	
	{"bytes=-10", utility::range_header::all, 10, true},  
	{"bytes=10-", 10, utility::range_header::all, true},  
	{"bytes=-", utility::range_header::all, utility::range_header::all, true},  
	{"bytes=*-10", utility::range_header::all, 10, true},  
	{"bytes=10-*", 10, utility::range_header::all, true},  
	{"bytes=*-*", utility::range_header::all, utility::range_header::all, true},  

	{"bytes=-100", utility::range_header::all, 100, true},  
	{"bytes=100-", 100, utility::range_header::all, true},  
	{"bytes=-", utility::range_header::all, utility::range_header::all, true},  
	{"bytes=*-100", utility::range_header::all, 100, true},  
	{"bytes=100-*", 100, utility::range_header::all, true},  
	{"bytes=*-*", utility::range_header::all, utility::range_header::all, true},  

	{"bytes=0-1", 0, 1, true},  
	{"bytes=0-10", 0, 10, true},  
	{"bytes=0-100", 0, 100, true},  
	{"bytes=0-1000", 0, 1000, true},  
	{"bytes=1000-10000", 1000, 10000, true} 

};

struct helper_2 {
	char const * range_header;
	boost::int64_t ex_beg_1;	
	boost::int64_t ex_end_1;
	boost::int64_t ex_beg_2;
	boost::int64_t ex_end_2;
	bool ex_result;
} test_data_case_2[] = {
	{"bytes=1-,-1", 1, utility::range_header::all, utility::range_header::all, 1, true},  
	{"bytes=*-1,1-*", utility::range_header::all, 1, 1, utility::range_header::all, true},  
	{"bytes=1-*,*-1", 1, utility::range_header::all, utility::range_header::all, 1, true},  
	
	{"bytes=0-1,0-1", 0, 1, 0, 1, true},  
	{"bytes=0-10,0-10", 0, 10, 0, 10, true},  
	{"bytes=0-100,0-100", 0, 100, 0, 100, true},  
	{"bytes=0-1000,0-1000", 0, 1000, 0, 1000, true},  
	{"bytes=1000-10000,1000-10000", 1000, 10000, 1000, 10000, true} 

};

struct helper_3 {
	char const * range_header;
} test_data_case_3[] = {
	{"bytes="}, 
	{"bytes=YYY-10"},
	{"bytes=10-YYY"},
	{"bytes=10-YYY,"},
	{"bytes=-,10-YYY"}
};

int test_main(int argc, char ** argv) 
{
	for (std::size_t it = 0; it < sizeof(test_data_case_1)/sizeof(test_data_case_1[0]); ++it) 
	{
		FIRST_TEST_CASE(test_data_case_1[it].range_header, 
			test_data_case_1[it].ex_begin, 
			test_data_case_1[it].ex_end,
			test_1,
			test_data_case_1[it].ex_result)
	}
	
	for (std::size_t it = 0; it < sizeof(test_data_case_2)/sizeof(test_data_case_2[0]); ++it) 
	{
		SECOND_TEST_CASE(test_data_case_2[it].range_header, 
			test_data_case_2[it].ex_beg_1, 
			test_data_case_2[it].ex_end_1,
			test_data_case_2[it].ex_beg_2, 
			test_data_case_2[it].ex_end_2,
			test_2,
			test_data_case_2[it].ex_result)
	}

	for (std::size_t it = 0; it < sizeof(test_data_case_3)/sizeof(test_data_case_3[0]); ++it) 
		THIRD_TEST_CASE(test_data_case_2[it].range_header, test_3); 



	return 0;
}

