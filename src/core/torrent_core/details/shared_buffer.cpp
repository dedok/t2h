#include "shared_buffer.hpp"

#include <boost/bind.hpp>

namespace t2h_core { namespace details {

boost::function<std::size_t(std::string const &)> shared_buffer_default_hasher = boost::bind(&boost::hash_value, _1);
/**
 * Public shared_buffer api
 */

shared_buffer::shared_buffer() 
	: torrents_(), lock_(), hasher_(shared_buffer_default_hasher) { }

shared_buffer::~shared_buffer() 
{ 
}

boost::tuple<bool, std::size_t> shared_buffer::add(torrent_ex_info_ptr info) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	std::size_t const hash_value = hasher_(info->torrent_params.save_path);	
	if (torrents_.find(hash_value) == torrents_.end()) {
		info->index = hash_value;
		torrents_[hash_value] = info;
		return boost::make_tuple(true, hash_value);
	}
	return boost::make_tuple(false, static_cast<std::size_t>(-1));
}


torrent_ex_info_ptr shared_buffer::get(std::string const & path) 
{	
	boost::lock_guard<boost::mutex> guard(lock_);
	std::size_t const hash_value = hasher_(path);
	return get_usafe(hash_value);
}

torrent_ex_info_ptr shared_buffer::get(std::size_t hash_value) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	return get_usafe(hash_value);
}

void shared_buffer::remove(std::size_t hash_value) 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	remove_usafe(hash_value);
}

void shared_buffer::remove(std::string const & path) 
{
	std::size_t const hash_value = hasher_(path);
	remove(hash_value);
}

/**
 * Private shared_buffer api
 */
torrent_ex_info_ptr shared_buffer::get_usafe(std::size_t hash_value) 
{
	if (torrents_.find(hash_value) != torrents_.end())
		return torrents_[hash_value];
	return torrent_ex_info_ptr();
}

void shared_buffer::remove_usafe(std::size_t hash_value) 
{
	torrents_type::iterator found;
	if ((found = torrents_.find(hash_value)) != torrents_.end())
		torrents_.erase(found);
}

boost::function<std::size_t(std::string const &)> 
	shared_buffer::get_hasher() const 
{
	boost::lock_guard<boost::mutex> guard(lock_);
	return hasher_;
}

void set_hasher(boost::function<std::size_t(std::string const &)> hasher) 
{
	/*  TODO Fail case : after then we set new hasher we must to rehash all map,
		otherwise UB(eg colisios) */
	boost::lock_guard<boost::mutex> guard(lock_);
	hasher_ = hasher;
}

} } // namespace t2h_core, details

