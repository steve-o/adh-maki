/* SNMP row handler.  Each column in the row can be a variable or an attribute.
 */

#ifndef __SNMP_ROW_HH__
#define __SNMP_ROW_HH__
#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

#include <rmc.hh>
#include <net-snmp.hh>

namespace adh {
namespace snmp {

	class object_t;
	class variable_t;
	class table_t;

	extern "C" {
		int handler (netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo, netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests);
	}
	

	class row_t :
		public RTRProxyManagedObjectServerClient,
		public RTRProxyManagedObjectClient
	{
	public:
		row_t (RTRProxyManagedObjectServer& server, const RTRProxyManagedObjectHandle& handle);
		row_t (RTRProxyManagedObjectPtr rmc_object);

		std::shared_ptr<variable_t> addVariable (table_t& table, std::shared_ptr<object_t> object);

/* observers */
		const char* instance();
		bool inSync();

	protected:
		friend class table_t;
		friend int handler (netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo, netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests);
/* SNMP columns */
		std::vector<std::shared_ptr<variable_t>> variables_;

/* RMC object associated with this table */
		RTRProxyManagedObjectPtr object_;

	public:
// Event processing -- RTRProxyManagedObjectServerClient
		virtual void processObjectServerError (RTRProxyManagedObjectServer& pmos) override;
                virtual void processObjectServerSync (RTRProxyManagedObjectServer& pmos) override;
                virtual void processObjectServerRootAdded (RTRProxyManagedObjectServer& pmos, const RTRProxyManagedObjectHandle& pmoh) override;
                virtual void processObjectServerRootRemoved (RTRProxyManagedObjectServer& pmos, const RTRProxyManagedObjectHandle& pmoh) override;

// Event processing -- RTRProxyManagedObjectClient
		virtual void processProxyManagedObjectError (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectSync (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectDeleted (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectInfo (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectInService (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectRecovering (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectWaiting (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectDead (const RTRProxyManagedObject& pmo) override;
		virtual void processProxyManagedObjectChildAdded (const RTRProxyManagedObject& pmo, const RTRProxyManagedObjectHandle& pmoh) override;
		virtual void processProxyManagedObjectChildRemoved (const RTRProxyManagedObject& pmo, const RTRProxyManagedObjectHandle& pmoh) override;
		virtual void processProxyManagedObjectVariableAdded (const RTRProxyManagedObject& pmo, const RTRProxyManagedVariableHandle& pmoh) override;
		virtual void processProxyManagedObjectVariableRemoved (const RTRProxyManagedObject& pmo, const RTRProxyManagedVariableHandle& pmoh) override;

	protected:
		RTRLinkedList<RTRProxyManagedObjectPtr> _pmoList;
		void push_back (const RTRProxyManagedObjectPtr obj);
	};

} /* namespace snmp */
} /* namespace adh */

#endif /* __SNMP_ROW_HH__ */

/* eof */
