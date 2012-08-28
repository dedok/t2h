#include "misc_utility.hpp"

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace utility {

static std::string const chars_set =  
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"1234567890";

std::string get_random_string(std::size_t dist_max) 
{
	std::string random_string;
	if (dist_max > chars_set.size() - 1)
		return std::string();

	boost::random::random_device device;
	boost::random::uniform_int_distribution<> index_dist(0, chars_set.size() - 1);
	
	random_string.resize(dist_max);
	for (std::size_t it = 0; it < dist_max; ++it) 
		random_string.at(it) = chars_set.at(index_dist(device));
	
	return random_string;
}

} // namespace utility

