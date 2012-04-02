/* SNMP agent, single session.
 */

#include "snmp_agent.hh"

/* nullptr requires GCC 4.6 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#else
#	include "nullptr.hh"
#endif

#include <unistd.h>
#include <fcntl.h>
#ifdef HAVE_EVENTFD
#include <sys/eventfd.h>
#endif

/* redirect namespace pollution */
#define U64 __netsnmp_U64

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* revert Net-snmp namespace pollution */
#undef U64
#undef LOG_INFO
#undef LOG_WARNING

boost::atomic<int> snmp::agent_t::ref_count_ (0);

class snmp::event_pump_t
{
public:
	event_pump_t (int fd) :
		fd_ (fd)
	{
	}

	void operator()()
	{
puts ("entr thread");
		assert (-1 != fd_);

		for (;;)
		{
			int fds = 0, block = 1;
			fd_set fdset;
			struct timeval timeout;

			FD_ZERO(&fdset);
			::snmp_select_info (&fds, &fdset, &timeout, &block);
			FD_SET(fd_, &fdset);
			fds = std::max (fd_, fds) + 1;
			fds = ::select (fds, &fdset, NULL, NULL, block ? NULL : &timeout);
			if (fds) {
				if (FD_ISSET(fd_, &fdset)) {
					break;
				}
				::snmp_read (&fdset);
			} else {
				::snmp_timeout();
			}
		}
puts ("exit thread");
	}

private:
	int fd_;
};

snmp::agent_t::agent_t (
	const char* application_name,
	bool is_agentx_subagent,
	const char* agentx_socket
	) :
	application_name_ (application_name),
	is_agentx_subagent_ (is_agentx_subagent),
#ifdef HAVE_EVENTFD
	fd_ (-1)
#else
	pipe_ { -1, -1 }
#endif
{
// cannot assign nullptr to std::string
	if (nullptr != agentx_socket)
		agentx_socket_ = agentx_socket;
}

snmp::agent_t::~agent_t (void)
{
	clear();
}

bool
snmp::agent_t::run (void)
{
/* Instance already running. */
	if (0 != ref_count_.fetch_add (1, boost::memory_order_relaxed))
		return true;

	if (is_agentx_subagent_)
	{
		if (!agentx_socket_.empty())
		{
			::netsnmp_ds_set_string (NETSNMP_DS_APPLICATION_ID,
						 NETSNMP_DS_AGENT_X_SOCKET,
						 agentx_socket_.c_str());
		}
		::netsnmp_ds_set_boolean (NETSNMP_DS_APPLICATION_ID,
					  NETSNMP_DS_AGENT_ROLE,
					  TRUE);
	}

	if (!filelog_.empty()) {
		::snmp_enable_filelog (filelog_.c_str(), 0);
	}

	if (0 != ::init_agent (application_name_.c_str())) {
		return false;
	}

/* MIB tables and respective handlers. */
	if (!init_MIB ())
		return false;

/* read config and parse mib */
	::init_snmp (application_name_.c_str());

	if (!is_agentx_subagent_)
	{
		if (0 != ::init_master_agent ()) {
			return false;
		}
	}

/* create notification channel */
#ifdef HAVE_EVENTFD
	fd_ = eventfd (0, EFD_NONBLOCK);
	if (-1 == fd_)
		return false;
#else
	if (-1 == pipe2 (pipe_, O_NONBLOCK))
		return false;
/* pass on read-end */
	const int fd_ = pipe_[0];
#endif

/* spawn thread to handle SNMP requests */
	event_pump_.reset (new snmp::event_pump_t (fd_));
	if (!(bool)event_pump_)
		return false;
	thread_.reset (new boost::thread (*event_pump_.get()));
	if (!(bool)thread_)
		return false;
	return true;
}

/* Terminate thread and free resources.
 */

void
snmp::agent_t::clear (void)
{
	if (1 != ref_count_.fetch_sub (1, boost::memory_order_release))
		return;
	if ((bool)thread_ && thread_->joinable()) {
#ifdef HAVE_EVENTFD
		assert (-1 != fd_);
		const uint64_t u = 1;
		write (fd_, &u, sizeof (u));
		thread_->join();
		close (fd_);
		fd_ = -1;
#else
		assert (-1 != pipe_[1]);
		const char one = '1';
		write (pipe_[1], &one, sizeof (one));
		thread_->join();
		close (pipe_[1]); pipe_[1] = -1;
		close (pipe_[0]); pipe_[0] = -1;
#endif
	}
	::snmp_shutdown (application_name_.c_str());
puts ("fin");
}

/* eof */
