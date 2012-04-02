/* SNMP object handler.
 */

#include "snmp_variable.hh"

#include "chromium/logging.hh"

#include "snmp_object.hh"
#include "snmp_row.hh"
#include "snmp_table.hh"

static
const char*
object_state_text (
	RTRProxyManagedObject::PMOState state
	)
{
	switch (state) {
	case RTRProxyManagedObject::Init: return "Init";
	case RTRProxyManagedObject::Normal: return "Normal";
	case RTRProxyManagedObject::ManualRecovery: return "Manual Recovery";
	case RTRProxyManagedObject::AutoRecovery: return "Auto Recovery";
	case RTRProxyManagedObject::Dead: return "Dead";
	default: return "Unknown";
	}
}

adh::snmp::variable_t::variable_t (
	table_t& table,
	row_t& row,
	std::shared_ptr<object_t> object
	) :
	table_ (table),
	object_ (object),
	long_value_ (0L)
{
	assert (RMC_ATTRIBUTE_INVALID != object_->attribute());

	if (table_.hasClassFilter())
	{
/* leave untransformed */
		object_id_ = object_->instanceId();
	}
	else
	{
/* prepend row instance */
		RTRObjectId context (row.instance());
		object_id_ = RTRObjectId (context.parent(), object_->instanceId().string());
	}
}

// Event forwarding
void
adh::snmp::variable_t::addRootObject (
	RTRProxyManagedObjectPtr rmc_object
	)
{
	LOG(INFO) << "addRootObject";

LOG(INFO) << "root: " << rmc_object->instanceId()
	<< " target: " << object_id_;

	assert (!table_.hasClassFilter());
	if (rmc_object->instanceId() == object_id_) {
LOG(INFO) << "Root is target RMC Object";
		rmc_object_ = rmc_object;
	}
	push_back (rmc_object);
	rmc_object->addClient (*this);
	if (RTRTRUE == rmc_object->inSync())
		processProxyManagedObjectSync (*rmc_object);
}

void
adh::snmp::variable_t::addClassObject (
	RTRProxyManagedObjectPtr rmc_object
	)
{
	LOG(INFO) << "addClassObject";

	assert (table_.hasClassFilter());

LOG(INFO) << "root: " << rmc_object->instanceId()
	<< " target: " << object_id_;

/* re-route the instanceId to the class object root */
	object_id_ = RTRObjectId (rmc_object->instanceId(), object_id_.string());

LOG(INFO) << "re-routing search to instanceId: " << object_id_;
LOG_IF(INFO, table_.hasClassFilter()) << "root class: " << rmc_object->classId().string();

	assert (rmc_object->classId().conformsTo (table_.classId()));
	assert (rmc_object->classId().equivalent (table_.classId()));

LOG(INFO) << "Root is target RMC Class";
	if (rmc_object->instanceId() == object_id_) {
LOG(INFO) << "Root is target RMC Object in class filter, name:" << rmc_object->name();
		rmc_object_ = rmc_object;
	}
	push_back (rmc_object);
	rmc_object->addClient (*this);
	if (RTRTRUE == rmc_object->inSync())
		processProxyManagedObjectSync (*rmc_object);
}

u_char
adh::snmp::variable_t::type()
{
	return object_->type();
}

/* is this SNMP object's RMC object valid and synchronised.
 */
bool
adh::snmp::variable_t::inSync()
{
	DCHECK (nullptr != object_);
	if (!object_->isObjectVariable())
		return true;
	if (nullptr == rmc_variable_)
		rmc_variable_ = rmc_object_->variableByName (object_->name().c_str());
	if (nullptr == rmc_variable_)
		return false;
/* 4.3.2.4. State, OutOfSync: inSync = False */
	if (!rmc_variable_->inSync())
		return false;
/* InSync_Error: inSync = True, error = True */
	if (rmc_variable_->error())
		return false;
/* InSync_Ok: inSync = True, error = False */
	return true;
}

/* returns pointer to underlying RMC native data type.
 */
const
void*
adh::snmp::variable_t::data()
{
	assert (inSync());
	assert (RMC_ATTRIBUTE_INVALID != object_->attribute());

	if (!object_->isObjectVariable()) {
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_OBJECT_STATE:
			return object_state_text (rmc_object_->state());
		case RMC_ATTRIBUTE_OBJECT_NAME:
			return rmc_object_->name().to_c();
		default:
			NOTIMPLEMENTED();
			break;
		}
	}

	assert ((bool)rmc_variable_);
	switch (rmc_variable_->type()) {
	case RTRProxyManagedVariableHandle::Numeric:
	{
		const RTRProxyManagedNumeric& numeric = *rmc_variable_;
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_ACTIVE_VALUE:
			long_value_ = numeric.value();
			return &long_value_;
		default:
			NOTIMPLEMENTED();
			break;
		}
		break;
	}

	case RTRProxyManagedVariableHandle::Gauge:
	{
		const RTRProxyManagedGauge& gauge = *rmc_variable_;
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_ACTIVE_VALUE:
			long_value_ = gauge.value();
			return &long_value_;
		default:
			NOTIMPLEMENTED();
			break;
		}
		break;
	}

	case RTRProxyManagedVariableHandle::String:
	{
		const RTRProxyManagedString& string = *rmc_variable_;
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_ACTIVE_VALUE:
			return string.value().to_c();
		default:
			NOTIMPLEMENTED();
			break;
		}
		break;
	}

	default:
		NOTIMPLEMENTED();
		break;
	}

	return nullptr;
}

/* returns size of underlying native RMC data type.
 */
size_t
adh::snmp::variable_t::size()
{
	assert (inSync());
	assert (RMC_ATTRIBUTE_INVALID != object_->attribute());

	if (!object_->isObjectVariable()) {
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_OBJECT_STATE:
			return strlen (object_state_text (rmc_object_->state()));
		case RMC_ATTRIBUTE_OBJECT_NAME:
			return rmc_object_->name().count();
		default:
			NOTIMPLEMENTED();
			break;
		}
	}

	assert ((bool)rmc_variable_);
	switch (rmc_variable_->type()) {
	case RTRProxyManagedVariableHandle::Numeric:
	case RTRProxyManagedVariableHandle::Gauge:
	{
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_ACTIVE_VALUE:
			return sizeof (long);
		default:
			NOTIMPLEMENTED();
			break;
		}
		break;
	}

	case RTRProxyManagedVariableHandle::String:
	{
		const RTRProxyManagedString& string = *rmc_variable_;
		switch (object_->attribute()) {
		case RMC_ATTRIBUTE_ACTIVE_VALUE:
			return string.value().count() - 1;
		default:
			NOTIMPLEMENTED();
			break;
		}
		break;
	}

	default:
		NOTIMPLEMENTED();
		break;
	}

	return 0;
}

bool
adh::snmp::variable_t::isObjectVariable()
{
	return object_->isObjectVariable();
}

bool
adh::snmp::variable_t::isIndex()
{
	return object_->isIndex();
}

// Event processing for RTRProxyManagedObjectClient
void
adh::snmp::variable_t::processProxyManagedObjectError(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectError: " << object.text();
}

void
adh::snmp::variable_t::processProxyManagedObjectSync(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectSync";

LOG(INFO) << "sync for " << object.instanceId().string();

/* only search for children if root object is not a match */
	if (nullptr == rmc_object_)
	{
LOG(INFO) << "processing children";
		auto it = object.childHandles();
		for (it.start(); !it.off(); it.forth())
		{
			auto child = object.childByName (it.item().name());
			if (child->error())
				break;
LOG(INFO) << "child: " << child->instanceId().string();

			if (!object_id_.isDescendant (child->instanceId()))
			{
LOG(INFO) << "ignoring non-descendent";
				continue;
			}
			if (child->instanceId() == object_id_) {
LOG(INFO) << "Child is target RMC Object";
				rmc_object_ = child;
LOG_IF(INFO, table_.hasClassFilter()) << "Child name: " << child->name();
			}
			push_back (child);
			child->addClient (*this);
			if (RTRTRUE == child->inSync())
				processProxyManagedObjectSync (*child);
		}
		return;
	}

	if (!isObjectVariable()) {
LOG(INFO) << "skipping as attribute";
		return;
	}

	auto jt = object.variableHandles();
	for (jt.start(); !jt.off(); jt.forth())
	{
LOG(INFO) << "var: " << jt.item().name()
	<< " target: " << object_->name();
		if (jt.item().name().to_c() != object_->name())
			continue;

// dumb as fucking balls
		rmc_variable_ = object.variableByName (object_->name().c_str());
LOG(INFO) << "found RMC variable: " << object_->name();
		break;
	}
}

void
adh::snmp::variable_t::processProxyManagedObjectDeleted(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectDeleted";
}

void
adh::snmp::variable_t::processProxyManagedObjectInfo(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectInfo";
}

void
adh::snmp::variable_t::processProxyManagedObjectInService(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectInService";
}

void
adh::snmp::variable_t::processProxyManagedObjectRecovering(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectRecovering";
}

void
adh::snmp::variable_t::processProxyManagedObjectWaiting(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectWaiting";
}

void
adh::snmp::variable_t::processProxyManagedObjectDead(
        const RTRProxyManagedObject& object
        )
{
	LOG(INFO) << "ObjectDead";
}

void
adh::snmp::variable_t::processProxyManagedObjectChildAdded(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedObjectHandle& handle
        )
{
	LOG(INFO) << "ObjectChildAdded";
}

void
adh::snmp::variable_t::processProxyManagedObjectChildRemoved(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedObjectHandle& handle
        )
{
	LOG(INFO) << "ObjectChildRemoved";
}

void
adh::snmp::variable_t::processProxyManagedObjectVariableAdded(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedVariableHandle& handle
        )
{
	LOG(INFO) << "ObjectVariableAdded";
}

void
adh::snmp::variable_t::processProxyManagedObjectVariableRemoved(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedVariableHandle& handle
        )
{
	LOG(INFO) << "ObjectVariableRemoved";
}

void
adh::snmp::variable_t::push_back (
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
