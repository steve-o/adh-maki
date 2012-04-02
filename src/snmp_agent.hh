/* SNMP agent, single session.
 */

#ifndef __SNMP_AGENT_HH__
#define __SNMP_AGENT_HH__
#pragma once

/* Boost noncopyable base class. */
#include <boost/utility.hpp>

/* Boost threading. */
#include <boost/thread.hpp>

/* Boost atomics. */
#include <boost/atomic.hpp>

namespace snmp
{
	class event_pump_t;

	class agent_t
		: boost::noncopyable
	{
	public:
		agent_t (const char* application_name, bool is_agentx_subagent, const char* agentx_socket);
		~agent_t();

		bool run();
		void clear();

		bool is_agentx_subagent_;
		std::string agentx_socket_;
		std::string filelog_;

	protected:
		virtual bool init_MIB() = 0;

	private:
/* SNMP application name, for finding configuration. */
		std::string application_name_;

/* SNMP event pump and thread. */
		std::unique_ptr<event_pump_t> event_pump_;
		std::unique_ptr<boost::thread> thread_;

/* Shutdown notification. */
#ifdef HAVE_EVENTFD
		int fd_;
#else
		int pipe_[2];
#endif

/* Only one session. */
		static boost::atomic<int> ref_count_;
	};

} /* namespace snmp */

#endif /* __SNMP_AGENT_HH__ */

/* eof */
