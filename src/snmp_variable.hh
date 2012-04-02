/* SNMP variable handler.
 */

#ifndef __SNMP_VARIABLE_HH__
#define __SNMP_VARIABLE_HH__
#pragma once

#include <memory>
#include <string>

#include <rmc.hh>

namespace adh {
namespace snmp {

	class object_t;
	class row_t;
	class table_t;

	enum {
		RMC_ATTRIBUTE_INVALID,
/* value of inSync variable */
		RMC_ATTRIBUTE_ACTIVE_VALUE = 1,
/* state of object */
		RMC_ATTRIBUTE_OBJECT_STATE,
/* name of object */
		RMC_ATTRIBUTE_OBJECT_NAME
	};

/* no need for RTRProxyManagedVariableClient notifications.
 */
	class variable_t :
		public RTRProxyManagedObjectClient
	{
	public:
/* cannot instantiate with RMC object & variable as both do not exist yet. */
		variable_t (table_t& table, row_t& row, std::shared_ptr<object_t> object);

		void addRootObject (RTRProxyManagedObjectPtr rmc_object);
		void addClassObject (RTRProxyManagedObjectPtr rmc_object);

/* observers */
		const RTRObjectId& instanceId()
		{
			return object_id_;
		}

		u_char type();
		const void* data();
		size_t size();
		bool isObjectVariable();
		bool isIndex();
		bool inSync();

	protected:
/* SNMP table */
		table_t& table_;
/* SNMP table column */
		std::shared_ptr<object_t> object_;

/* RMC object reference by this SNMP variable */
		RTRObjectId object_id_;
		RTRProxyManagedObjectPtr rmc_object_;
		RTRProxyManagedVariablePtr rmc_variable_;

/* Workaround for observing a reference to a simple type */
		long long_value_;

	public:
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

#endif /* __SNMP_VARIABLE_HH__ */

/* eof */
