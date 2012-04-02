/* SNMP row handler.
 */

#include "snmp_row.hh"

#include "chromium/logging.hh"

#include "snmp_variable.hh"

/* start subscribing to all objects from the object server.
 */
adh::snmp::row_t::row_t (
	RTRProxyManagedObjectServer& server,
	const RTRProxyManagedObjectHandle& handle
	)
{
/* take reference to handle object for lifetime of this object. */
	object_ = server.object (handle);
}

adh::snmp::row_t::row_t (
	RTRProxyManagedObjectPtr rmc_object
	) :
	object_ (rmc_object)
{
}

std::shared_ptr<adh::snmp::variable_t>
adh::snmp::row_t::addVariable (
	table_t& table,
	std::shared_ptr<object_t> object
	)
{
	assert ((bool)object);
	auto variable = std::make_shared<variable_t> (table, *this, object);
	variables_.push_back (variable);
	return variable;
}

// Event processing for RTRProxyManagedObjectServerClient
void
adh::snmp::row_t::processObjectServerError(
	RTRProxyManagedObjectServer& server
	)
{
	LOG(INFO) << "ServerError";
}

void
adh::snmp::row_t::processObjectServerSync(
	RTRProxyManagedObjectServer& server
	)
{
	LOG(INFO) << "ServerSync";
	auto it = server.roots();
	for (it.start(); !it.off(); it.forth())
		processObjectServerRootAdded (server, it.item());
}

void
adh::snmp::row_t::processObjectServerRootAdded(
	RTRProxyManagedObjectServer& server,
	const RTRProxyManagedObjectHandle& handle
	)
{
	LOG(INFO) << "RootAdded";
	auto object = server.object (handle);
	object->addClient (*this);
	push_back (object);
	if (RTRTRUE == object->inSync())
		processProxyManagedObjectSync (*object);
}

void
adh::snmp::row_t::processObjectServerRootRemoved(
	RTRProxyManagedObjectServer& server,
	const RTRProxyManagedObjectHandle& handle
	)
{
	LOG(INFO) << "RootRemoved";
}

// Event processing for RTRProxyManagedObjectClient
void
adh::snmp::row_t::processProxyManagedObjectError(
	const RTRProxyManagedObject& object
	)
{
	LOG(INFO) << "ObjectError: " << object.text();
}

void
adh::snmp::row_t::processProxyManagedObjectSync(
	const RTRProxyManagedObject& object
	)
{
	LOG(INFO) << "ObjectSync";
#if 0
	auto it = object.childHandles();
	for (it.start(); !it.off(); it.forth())
	{
		auto child = object.childByName (it.item().name());
		if (child->error())
			break;
		push_back (child);
		child->addClient (*this);
		if (RTRTRUE == child->inSync())
			processProxyManagedObjectSync (*child);
	}
#endif
}

void
adh::snmp::row_t::processProxyManagedObjectDeleted(
	const RTRProxyManagedObject& object
	)
{
	LOG(INFO) << "ObjectDeleted";
}

void
adh::snmp::row_t::processProxyManagedObjectInfo(
	const RTRProxyManagedObject& object
	)
{
	LOG(INFO) << "ObjectInfo";
}

void
adh::snmp::row_t::processProxyManagedObjectInService (
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectInService";
}

void
adh::snmp::row_t::processProxyManagedObjectRecovering (
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectRecovering";
}

void
adh::snmp::row_t::processProxyManagedObjectWaiting (
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectWaiting";
}

void
adh::snmp::row_t::processProxyManagedObjectDead (
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectDead";
}

void
adh::snmp::row_t::processProxyManagedObjectChildAdded (
        const RTRProxyManagedObject& object,
        const RTRProxyManagedObjectHandle& handle
        )
{
        LOG(INFO) << "ObjectChildAdded";
}

void
adh::snmp::row_t::processProxyManagedObjectChildRemoved (
        const RTRProxyManagedObject& object,
        const RTRProxyManagedObjectHandle& handle
        )
{
        LOG(INFO) << "ObjectChildRemoved";
}

void
adh::snmp::row_t::processProxyManagedObjectVariableAdded (
        const RTRProxyManagedObject& object,
        const RTRProxyManagedVariableHandle& handle
        )
{
        LOG(INFO) << "ObjectVariableAdded";
}

void
adh::snmp::row_t::processProxyManagedObjectVariableRemoved (
        const RTRProxyManagedObject& object,
        const RTRProxyManagedVariableHandle& handle
        )
{
        LOG(INFO) << "ObjectVariableRemoved";
}

/* return the RMC instanceID or context for the row major object. */
const char*
adh::snmp::row_t::instance()
{
	assert (nullptr != object_);
	assert (object_->instanceId().numberOfElements() >= 2);
	return object_->instanceId().to_c();
}

/* is the row's RMC object valid and synchronised.
 */
bool
adh::snmp::row_t::inSync()
{
	if (nullptr == object_)
		return false;
	if (!object_->inSync())
		return false;
	if (object_->error())
		return false;
	return true;
}

void
adh::snmp::row_t::push_back (
	const RTRProxyManagedObjectPtr obj
	)
{
	RTRProxyManagedObjectPtr objPtr;
        RTRBOOL done = RTRFALSE;  //used to maintain correct managed object tree structure

        RTRProxyManagedObjectPtr *nPtr =  new RTRProxyManagedObjectPtr;
        *nPtr = obj;
        if ( _pmoList.empty() )
        {
                _pmoList.addRight( nPtr );
        }
        else
        {
                for ( _pmoList.start(); !_pmoList.off(); _pmoList.forth() )
                {
                        objPtr = *(_pmoList.item());

                        if ( obj->instanceId().parent() == objPtr->instanceId() )
                        {
                                _pmoList.addRight( nPtr );
                                done = RTRTRUE;
                                break;
                        }
                }
                if ( !done )
                {
                        _pmoList.start();
                        _pmoList.addRight( nPtr );
                }
        }
}

/* eof */
