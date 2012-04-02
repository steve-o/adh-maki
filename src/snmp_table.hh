/* SNMP table handler.  Each row of the table is an RMC object.
 */

#ifndef __SNMP_TABLE_HH__
#define __SNMP_TABLE_HH__
#pragma once

#include <list>
#include <map>
#include <memory>
#include <vector>
#include <string>

#include <rmc.hh>
#include <net-snmp.hh>

namespace adh {
namespace snmp {

	class row_t;
	class object_t;
	class context_t;

	class table_t :
		public RTRProxyManagedObjectServerPoolClient,
		public RTRProxyManagedObjectServerClient,
		public RTRProxyManagedObjectClient
	{
	public:
		table_t (const std::vector<oid>& oid, const std::string& name, const std::string& class_filter);

		bool addRow (RTRProxyManagedObjectServer& server, const RTRProxyManagedObjectHandle& handle);
		bool addRow (RTRProxyManagedObjectPtr rmc_object);
		bool addObject (oid oid, const RTRObjectId& instanceId, int object_attribute, const std::string& name, bool is_index, u_char type);
		int registerSnmpTable();

		bool hasClassFilter()
		{
			return !class_filter_.empty();
		}

		const RTRObjectId& classId()
		{
			return classId_;
		}

		Netsnmp_First_Data_Point getFirstDataPoint;
		Netsnmp_Next_Data_Point getNextDataPoint;
		Netsnmp_Free_Loop_Context freeLoopContext;
		Netsnmp_Node_Handler processSnmpQuery;

	protected:
		std::vector<oid> oid_;
		std::string table_name_;
		std::string class_filter_;
		RTRObjectId classId_;

/* rows of table */
		std::list<std::shared_ptr<row_t>> rows_;
/* columns of table */
		std::vector<std::shared_ptr<object_t>> objects_;
/* class directory */
		RTRProxyManagedObjectClassDirectoryPtr directory_;
/* managed applications */
		std::multimap<RTRApplicationId, RTRProxyManagedObjectHandle> root_map_;

		bool addIndexes (netsnmp_table_registration_info* table_info);
		bool setIndexedValues (netsnmp_variable_list* idx, std::shared_ptr<row_t> row);
		friend class context_t;

	public:
// Event processing -- RTRProxyManagedObjectServerPoolClient
                virtual void processProxyManagedObjectServerAdded (RTRProxyManagedObjectServerPool& pmosp, RTRProxyManagedObjectServer& pmos) override;
                virtual void processProxyManagedObjectServerRemoved (RTRProxyManagedObjectServerPool& pmosp, RTRProxyManagedObjectServer& pmos) override;

// Event processing -- RTRProxyManagedObjectServerClient
                virtual void processObjectServerError (RTRProxyManagedObjectServer& pmos) override;
                virtual void processObjectServerSync (RTRProxyManagedObjectServer& pmos) override;
                virtual void processObjectServerRootAdded (RTRProxyManagedObjectServer& pmos, const RTRProxyManagedObjectHandle& pmoh) override;
                virtual void processObjectServerRootRemoved (RTRProxyManagedObjectServer& pmos, const RTRProxyManagedObjectHandle& pmoh) override;

// Event processing for RTRProxyManagedObjectClient
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

#endif /* __SNMP_TABLE_HH__ */

/* eof */
