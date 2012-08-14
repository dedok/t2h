#ifndef EVENT_HPP_INCLUDED
#define EVENT_HPP_INCLUDED

#include "events_policy.hpp"
#include "basic_events.hpp"

template <class Sink>
struct event_exchange_queue_binder 
	: public basic_event_binder<event_policy::exchange_events_queue<Sink> >
{ };

template <class Sink>
struct event_exchange_queue 
	: public basic_event_source<event_policy::exchange_events_queue<Sink> >
{ };

template <class Sink>
struct direct_call_binder 
	: public basic_event_binder<event_policy::direct_call<Sink> >
{ };

template <class Sink>
struct direct_call 
	: public basic_event_source<event_policy::direct_call<Sink> >
{ };


#endif

