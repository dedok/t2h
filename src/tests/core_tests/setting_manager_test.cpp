#include "setting_manager.hpp"

#include <fstream>
#include <boost/test/minimal.hpp>
#include <boost/filesystem/path.hpp>

#define CHECK_ENTITY(type, x , x1) 													\
do {																				\
	type t = x;																		\
	std::cout << "Result of [ " << #x << #x1 << " ] : " << t << std::endl;			\
	BOOST_CHECK(t x1);																\
} while(0);

#define DEFAULT_AFTER_INIT_TEST_CASES(x)																				\
CHECK_ENTITY(std::size_t, x->get_value<std::size_t>("workers"), == envt::workers_number)								\
CHECK_ENTITY(std::string, x->get_value<std::string>("server_addr"), == envt::server_addr)								\
CHECK_ENTITY(std::string, x->get_value<std::string>("server_port"), == envt::server_port)								\
CHECK_ENTITY(std::string, x->get_value<std::string>("doc_root"), == envt::doc_root)										\
CHECK_ENTITY(std::size_t, x->get_value<std::size_t>("tc_port_end"), == envt::tc_port_end)								\
CHECK_ENTITY(std::size_t, x->get_value<std::size_t>("tc_port_start"), == envt::tc_port_start)							\
CHECK_ENTITY(std::size_t, x->get_value<std::size_t>("tc_max_async_download_size"), == envt::tc_max_async_download_size)	\
CHECK_ENTITY(std::size_t, x->get_value<std::size_t>("tc_max_alert_wait_time"), == envt::tc_max_alert_wait_time)			\
CHECK_ENTITY(std::string, x->get_value<std::string>("tc_root"), == envt::tc_root)

namespace envt {

static std::size_t const workers_number = 4;
static std::string const server_addr = "127.0.0.1";
static std::string const server_port = "8080";
static boost::filesystem::path const doc_root = "test/path";
static std::size_t const tc_port_start = 6881;
static std::size_t const tc_port_end = 6889;
static std::size_t const tc_max_alert_wait_time = 4;
static std::size_t const tc_max_async_download_size = 5000000;
static std::string const tc_root = "test/path";

static boost::filesystem::path const path_to_config = boost::filesystem::current_path() / "test_config.json";

static char const * json_config = 
"{\n"
"\"workers\" : \"4\",\n"
"\"server_port\" : \"8080\",\n"
"\"server_addr\" : \"127.0.0.1\",\n" 
"\"doc_root\" : \"test/path\",\n" 
"\"tc_port_start\" : \"6881\",\n"
"\"tc_port_end\" : \"6889\", \n"
"\"tc_max_alert_wait_time\" : \"4\", \n"
"\"tc_max_async_download_size\" : \"5000000\", \n"
"\"tc_root\" : \"test/path\" "
"\n}";

static char const * json_config_2 =
"{\n"
"\"workers\" : \"4\",\n"
"\"server_port\" : \"8080\",\n"
"\"server_addr\" : \"127.0.0.1\",\n" 
"\"doc_root\" : \"test/path\",\n" 
"\"tc_port_start\" : \"6881\",\n"
"\"tc_port_end\" : \"6889\", \n"
"\"tc_max_alert_wait_time\" : \"4\", \n"
"\"tc_max_async_download_size\" : \"5000000\""
"\n}";

void prepare_test_envt(); 
void clear_test_envt();

}

static void panic(std::string const & message) 
{
	envt::clear_test_envt();
	throw message;
}

int test_main(int argc, char ** argv)
try
{
	using t2h_core::setting_manager;
	using t2h_core::setting_manager_ptr;

	envt::prepare_test_envt();
	setting_manager_ptr smp = setting_manager::shared_manager();	
	if (!smp) 
		panic("Setting manager pointer not valid");
	
	smp->load_config(envt::path_to_config);
	if (!smp->config_is_well())
		panic(std::string("Load config failed, with reason : " + smp->get_last_error()));
	DEFAULT_AFTER_INIT_TEST_CASES(smp)	
	
	smp->init_config(envt::json_config);
	if (!smp->config_is_well())
		panic(std::string("Load config failed, with reason : " + smp->get_last_error()));
	DEFAULT_AFTER_INIT_TEST_CASES(smp)	
	
	smp->init_config(envt::json_config_2);
	if (!smp->config_is_well())
		panic(std::string("Load config failed, with reason : " + smp->get_last_error()));
	CHECK_ENTITY(std::string, smp->get_value<std::string>("tc_root"), == smp->get_value<std::string>("doc_root"));

	envt::clear_test_envt();
	return 0;
}
catch (std::exception const & expt) 
{
	panic(std::string("Exeption : ") + expt.what());
	envt::clear_test_envt();
	return 1;
}
catch(...) 
{
	envt::clear_test_envt();
	return -1;
}

void envt::prepare_test_envt() 
{
	std::ofstream file(path_to_config.string().c_str());
	file << envt::json_config;	
}

void envt::clear_test_envt() 
{
	boost::system::error_code err;
	boost::filesystem::remove(path_to_config, err);
}

