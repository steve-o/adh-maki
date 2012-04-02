/* ADH-SNMP agent, single session.
 */

#ifndef __ADH_AGENT_HH__
#define __ADH_AGENT_HH__
#pragma once

#include <list>

#include <net-snmp.hh>
#include <rmc.hh>

#include "snmp_agent.hh"

namespace chromium {
	class DictionaryValue;
} /* namespace chromium */

namespace rmc {
	class monitor_t;
	class application_t;
} /* namespace rmc */

namespace adh
{
	namespace snmp {
		class table_t;
		class context_t;
	} /* namespace snmp */

	class agent_t :
		public ::snmp::agent_t
	{
	public:
		typedef ::snmp::agent_t super;
		agent_t (bool is_agentx_subagent, const char* agentx_socket);

		bool addServerPool (std::shared_ptr<RTRProxyManagedObjectServerPool> pool);

	protected:
		virtual bool init_MIB() override;
		bool createSnmpRoot (chromium::DictionaryValue* dict_val);
		bool createSnmpTable (chromium::DictionaryValue* dict_val);
		bool createSnmpObject (std::shared_ptr<snmp::table_t> table, chromium::DictionaryValue* dict_val);

		std::vector<oid> root_oid_;
		std::list<std::shared_ptr<snmp::table_t>> tables_;

/* one monitor to monitor them all */
		std::shared_ptr<rmc::monitor_t> monitor_;
/* one application per monitored instance */
		std::list<std::shared_ptr<rmc::application_t>> applications_;
	};

} /* namespace adh */

#endif /* __ADH_AGENT_HH__ */

/* eof */
