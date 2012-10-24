#include "partial_content_reply.hpp"

#include <fstream>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/minimal.hpp>

#if defined(ENABLE_DEBUG_OUTPUT)
#	define PRINT_BUFFER print_buffer();
#else
#	define PRINT_BUFFER
#endif

#define RUN_TEST(x, fs) 							\
do { 												\
	reset_envt(); 									\
	create_binary_data(test_file_path, fs); 		\
	BOOST_CHECK(x);									\
	PRINT_BUFFER									\
} while(0)

namespace {

utility::http_reply::buffer_type buffer;
utility::http_reply * pc_reply = NULL;
boost::filesystem::path const test_file_path = "http_reply_test_data";

#if defined ENABLE_DEBUG_OUTPUT
inline static void print_(char a) 
	{ std::cerr << a; }

inline static void print_buffer() 
	{ std::for_each(buffer.begin(), buffer.end(), print_); std::cerr << std::endl; }
#endif

static inline void reset_envt() 
{
	boost::system::error_code err;
	boost::filesystem::remove(test_file_path, err);
	if (pc_reply) 
		{ delete pc_reply; pc_reply = NULL; }
	buffer.clear();
}

static void create_binary_data(boost::filesystem::path const & path, boost::int64_t fsize) 
{
	std::ofstream stream(path.string().c_str(), 
		std::ios::out | std::ios::binary | std::ios::app);	
	for (std::size_t it = 0; it != (std::size_t)fsize; ++it) {
		srand (it * time(NULL));
		char byte[1] = { (char)rand() };
		stream.write(byte, 1);
	}
}

static void fill_data_from_file(
	utility::http_reply::buffer_type & buf_ref, 
	boost::int64_t bs, 
	boost::int64_t be, 
	boost::int64_t fs) 
{
	if (be == bs || be > fs) return;
	std::size_t const read_size = be < fs ? (1 * (be - bs)) + 1 : fs;
	buf_ref.resize(read_size);
	std::ifstream stream(test_file_path.string().c_str(), std::ios::in | std::ios::binary);
	stream.seekg((std::streamoff)bs, std::ios::beg);
	stream.readsome(&buf_ref.at(0), buf_ref.size());
	std::cerr << "Readed : " << stream.gcount() << std::endl;
}

} // namespace

static bool partial_content_test(boost::int64_t bs, boost::int64_t be, boost::int64_t fs) 
{	
	using namespace utility;
	using namespace t2h_core;

	http_reply::buffer_type data_from_file;
	details::partial_content_reply_param const pc_params = 
		details::create_partial_content_param(test_file_path, fs, bs, be);
	pc_reply = new details::partial_content_reply(buffer, pc_params);
	
	if (!pc_reply->do_formatting_reply()) {
		std::cerr << "Creating reply failed" << std::endl;
		return false;
	}
		
	fill_data_from_file(data_from_file, bs, be, fs);
	for (http_reply::buffer_type::iterator first = buffer.begin(), last = buffer.end();
		first != last; 
		++first) 
	{
		if (*first == '\n') {
			http_reply::buffer_type::iterator pos = first; 
			pos++; pos++; 
			if (*pos == '\n') {
				std::size_t elem_counter = 0; 
				++pos;
				for (http_reply::buffer_type::iterator dff_first = data_from_file.begin();
					pos != last && dff_first != data_from_file.end(); 
					++pos, ++dff_first) 
				{	
					++elem_counter;
					if (*pos != *dff_first) {
						std::cerr << "Diff detected at pos : " << elem_counter << std::endl;
						return false;
					}
				} // for
				break;
			} // if
		} // if
	} // for

	delete pc_reply; pc_reply = NULL;	
	return true;
}

int test_main(int argc, char ** argv) 
{
	try 
	{
		// Good data testing
		RUN_TEST(partial_content_test(0, 999, 1000), 1000);
		RUN_TEST(partial_content_test(9, 14, 1000), 1000);
		RUN_TEST(partial_content_test(998, 999, 1000), 1000);
		RUN_TEST(partial_content_test(1, 998, 1000), 1000);
		RUN_TEST(partial_content_test(998, 998, 1000), 1000);
		RUN_TEST(partial_content_test(999, 999, 1000), 1000);
		reset_envt();
	}
	catch (std::exception const & expt) 
	{
		reset_envt();
		std::cerr << "Exception : " << expt.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

