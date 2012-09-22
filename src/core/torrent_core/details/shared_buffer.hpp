#ifndef SHARED_BUFFER_HPP_INCLUDED
#define SHARED_BUFFER_HPP_INCLUDED

#include "torrent_core_macros.hpp"
#include "torrent_info.hpp"

#include <map>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/functional/hash.hpp>

namespace t2h_core { namespace details {

class shared_buffer;

inline static std::size_t hash_function(std::string const & str) { return boost::hash_value(str); } 

template <class Function> 
void for_each(shared_buffer const & sb, Function function);

class shared_buffer {
public :
	typedef std::map<std::size_t, torrent_ex_info_ptr> torrents_type; 
	
	shared_buffer();
	~shared_buffer();
	
//	boost::tuple<bool, torrent_ex_info_ptr> add(libtorrent::torrent_handle & handle);
	boost::tuple<bool, std::size_t> add(torrent_ex_info_ptr info);

	torrent_ex_info_ptr get(std::string const & path);
	torrent_ex_info_ptr get(std::size_t hash_value);

	void remove(std::string const & path);
	void remove(std::size_t hash_value);
	
	template <class Function> 
	friend void for_each(shared_buffer & sb, Function function);

	boost::function<std::size_t(std::string const &)> get_hasher() const;
	void set_hasher(boost::function<std::size_t(std::string const &)> hasher);
	
private :
	torrent_ex_info_ptr get_usafe(std::size_t hash_value);
	void remove_usafe(std::size_t hash_value);

	torrents_type torrents_;	
	boost::mutex mutable lock_;
	boost::function<std::size_t(std::string const &)> hasher_;

};

template <class Function> 
void for_each(shared_buffer & sb, Function function)
{
	boost::lock_guard<boost::mutex> guard(sb.lock_);
	std::for_each(sb.torrents_.begin(), sb.torrents_.end(), function);
}

} } // namespace t2h_core, details

#endif

