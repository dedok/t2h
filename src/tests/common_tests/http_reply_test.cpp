#include "http_utility.hpp"

#include <fstream>
#include <boost/bind.hpp>

#define TEST_USE_B_TEST_FRMW
#if defined(TEST_USE_B_TEST_FRMW)
#define ENTRY_POINT test_main
#include <boost/test/minimal.hpp>
#else
#define ENTRY_POINT main
#define BOOST_CHECK(x) if (x) { /*...*/ } 
#define BOOST_FAIL(x) std::cerr << x << std::endl;
#endif

namespace {

utility::http_reply::buffer_type buffer;
utility::http_reply reply(buffer);

boost::filesystem::path const test_file_path = "http_reply_test_data";
boost::filesystem::path const test_file_path_tmp = "http_reply_test_data_tmp";

static inline void reset_envt() 
{
	boost::system::error_code err;
	boost::filesystem::remove(test_file_path, err);
	boost::filesystem::remove(test_file_path_tmp, err);
	reply.set_no_auto_generate_headers(false);
	reply.set_status(utility::http_reply::bad_gateway);
	buffer.clear();
}

static bool compare_file_with_buffer_partial(
	boost::filesystem::path const & path, 
	utility::http_reply::buffer_type const & buff,
	boost::int64_t first, boost::int64_t last) 
{
	utility::http_reply::buffer_type fbuf;
	fbuf.resize(buff.size());
	std::ifstream stream(path.string().c_str(), std::ios::in | std::ios::binary);
	stream.seekg(first, std::ios_base::beg);
	stream.read(&fbuf.at(0), fbuf.size());
	return std::equal(fbuf.begin(), fbuf.end(), buff.begin());
}

static bool compare_file_with_buffer(
	boost::filesystem::path const & path, 
	utility::http_reply::buffer_type const & buff) 
{
	boost::int64_t fsize = 0;
	if ((fsize = boost::filesystem::file_size(path)) != (boost::int64_t)buff.size())  
		return false;
	return compare_file_with_buffer_partial(path, buff, 0, fsize);
}

static void create_binary_data(boost::filesystem::path const & path, boost::int64_t fsize) 
{
	std::ofstream stream(path.string().c_str(), 
		std::ios::out | std::ios::binary | std::ios::app);	
	for (std::size_t it = 0; it != (std::size_t)fsize; ++it)
		stream << it % 2;
} 

static bool test_case_1(boost::int64_t offset_, boost::int64_t fsize_) 
{
	using namespace utility;

	http_reply::buffer_type second_buff;			
	boost::int64_t fsize = 0, hi = 0, low = 0, offset = offset_;	

	reply.set_no_auto_generate_headers(true);
	create_binary_data(test_file_path, fsize_);
	
	fsize = boost::filesystem::file_size(test_file_path);
	std::ofstream stream(test_file_path_tmp.string().c_str(), std::ios::out | std::ios::binary);
	for (;;) {
		if (fsize <= 0) break;	
		hi = hi + offset;
		http_reply::formating_result result = 
			reply.format_partial_content(test_file_path, low, hi);
		if (result != http_reply::formating_succ)
			return false;
		low = low + offset; 
		fsize -= offset;
		stream.write(&buffer.at(0), buffer.size());
		for (http_reply::buffer_type::const_iterator it = buffer.begin(); 
			it != buffer.end(); 
			++it) 
		{
			second_buff.push_back(*it);
		}
		reply.reset_buffer();
	}
	stream.close();
	
	return compare_file_with_buffer(test_file_path_tmp, second_buff);
}

static bool test_case_N(boost::int64_t fsize, 
	boost::int64_t from, boost::int64_t to, 
	bool no_gen_headers, bool no_last_comp, bool no_deb_out) 
{
	using namespace utility;

	reply.set_no_auto_generate_headers(no_gen_headers);

	create_binary_data(test_file_path, fsize);
	http_reply::formating_result result =
		reply.format_partial_content(test_file_path, from, to);
	
	if (result != http_reply::formating_succ)
		return false;
	
	if (!no_deb_out) {	
		std::copy(buffer.begin(), buffer.end(),
    	       std::ostream_iterator<char>(std::cout));
		std::cout << std::endl;	
	}

	if (!no_last_comp)
		return compare_file_with_buffer_partial(test_file_path, buffer, from, 
		to <= 0 ? boost::filesystem::file_size(test_file_path) : to);
	return true;
}

static bool test_case_2(boost::int64_t fsize, boost::int64_t from, boost::int64_t to) 
{
	return test_case_N(fsize, from, to, true, false, true);	
}

static bool test_case_3(boost::int64_t fsize, boost::int64_t from, boost::int64_t to) 
{
	return test_case_N(fsize, from, to, true, false, true);
}

static bool test_case_4(boost::int64_t fsize, boost::int64_t from, boost::int64_t to) 
{
	return test_case_N(fsize, from, to, false, true, false);
}

} // namesapce

int ENTRY_POINT(int argc, char ** argv) 
{
	for (std::size_t it = 1; it != 100; ++it) {		
		reset_envt();
		bool const state = test_case_1(10 * it , 100 * it);
		BOOST_CHECK(state == true);
	}

	for (std::size_t it = 0; it < 1000; ++it) {
		reset_envt();
		BOOST_CHECK(test_case_2(1000, it, 1000));
	}
	
	reset_envt();
	BOOST_CHECK(test_case_3(1000, 0, -1));
	
	reset_envt();
	BOOST_CHECK(test_case_4(1000, 0, 99));

	reset_envt();
	return 0;
}

