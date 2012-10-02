#include "t2h.h"

#include <limits>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/test/minimal.hpp>

/**
 * Test helpers
 */

#define MAX_TYPE(x) std::numeric_limits<x>::max()

#define PRINT_TYPE_SIZE(x) \
	std::cerr << #x << " = " << MAX_TYPE(x) << std::endl;

#define CHECK_ENTITY(type, x , x1)                                                  \
do {                                                                                \
	type t = static_cast<type>(x);                                                  \
	std::cerr << "Result of [ " << #x << #x1 << " ] : " << t << std::endl;          \
	BOOST_CHECK(t x1);                                                              \
} while(0);

#define UINT_MAX_CHECK_ENTITY(x, x1)		\
do {										\
	CHECK_ENTITY(boost::uintmax_t, x, x1)	\
} while(0);		

/**
 * Test entry point
 */
int test_main(int argc, char ** argv) 
{
	std::cerr << "Type size info : " << std::endl;
#if !defined(_WIN32) || !defined(_WIN64)
	PRINT_TYPE_SIZE(ssize_t)
#endif
	PRINT_TYPE_SIZE(std::size_t)
	PRINT_TYPE_SIZE(unsigned int)
	PRINT_TYPE_SIZE(size_t)
	
	UINT_MAX_CHECK_ENTITY(MAX_TYPE(std::size_t), == MAX_TYPE(T2H_SIZE_TYPE))
	UINT_MAX_CHECK_ENTITY(MAX_TYPE(size_t), == MAX_TYPE(T2H_SIZE_TYPE))
	UINT_MAX_CHECK_ENTITY(MAX_TYPE(unsigned int), <= MAX_TYPE(T2H_SIZE_TYPE))
	
	return 0;
}

