/* SNMP table handler.
 */

#include "snmp_table.hh"

#include <algorithm>

#include "chromium/logging.hh"

#include "snmp_object.hh"
#include "snmp_variable.hh"
#include "snmp_row.hh"

namespace adh {
namespace snmp {
/* Context during a SNMP query, lock on global list of stitch_t objects and iterator.
 */
class context_t
{
public:
	context_t (table_t& table_) :
		begin (table_.rows_.begin()),
		end (table_.rows_.end()),
		it (begin)
	{
	}

	bool empty()
	{
		return (begin == end);
	}

	std::list<std::shared_ptr<row_t>>::iterator begin, end, it;

/* SNMP agent is not-reentrant, ignore locking. */
	static std::list<std::shared_ptr<context_t>> global_list;
};

std::list<std::shared_ptr<context_t>> context_t::global_list;

} /* namespace smp */
} /* namespace adh */

//namespace { /* anonymous namespace */

//static
int
adh::snmp::handler (
	netsnmp_mib_handler*		handler,
	netsnmp_handler_registration*	reginfo,
	netsnmp_agent_request_info*	reqinfo,
	netsnmp_request_info*		requests
	)
{
	assert (nullptr != handler);
#if 0
	auto table = static_cast<adh::snmp::table_t*> (handler->myvoid);
	assert (nullptr != table);

	return table->processSnmpQuery (handler, reginfo, reqinfo, requests);
#else
	LOG(INFO) << "handler";
	assert (nullptr != reginfo);
	assert (nullptr != reqinfo);
	assert (nullptr != requests);

	switch (reqinfo->mode) {

/* Read-support (also covers GetNext requests) */
	case MODE_GET:
	{
		for (netsnmp_request_info* request = requests;
		     request;
		     request = request->next)
		{
			const adh::snmp::row_t* row = static_cast<adh::snmp::row_t*>(netsnmp_extract_iterator_context (request));
			if (nullptr == row) {
				LOG(INFO) << "nullptr row";
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHINSTANCE);
				continue;
			}

			netsnmp_variable_list* var = request->requestvb;
			netsnmp_table_request_info* table_info = netsnmp_extract_table_info (request);
			if (nullptr == table_info) {
				LOG(INFO) << "nullptr table_info";
				snmp_log (__netsnmp_LOG_ERR, "handler: empty table request info.\n");
				continue;
			}

			auto variable = row->variables_[table_info->colnum - 1];
			if (!(bool)variable) {
				LOG(INFO) << "nullptr variable";
				snmp_log (__netsnmp_LOG_ERR, "handler: unknown column.\n");
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHOBJECT);
				break;
			}

			LOG(INFO) << "setting var";
			snmp_set_var_typed_value (var, variable->type(), variable->data(), variable->size());
			LOG(INFO) << "var set";
		}
		break;
	}

	default:
		LOG(INFO) << "unhandled request mode";
		snmp_log (__netsnmp_LOG_ERR, "handler: unsupported mode.\n");
		break;
	}
	return SNMP_ERR_NOERROR;
#endif
}

namespace { /* anonymous namespace */

static
netsnmp_variable_list*
get_first_data_point (
	void**			my_loop_context,	/* valid through one query of multiple "data points" */
	void**			my_data_context,	/* answer blob which is passed to handler() */
	netsnmp_variable_list*	put_index_data,		/* answer */
	netsnmp_iterator_info*	mydata			/* iinfo on init() */
	)
{
	assert (nullptr != mydata);
	auto table = static_cast<adh::snmp::table_t*> (mydata->myvoid);
	assert (nullptr != table);

	return table->getFirstDataPoint (my_loop_context, my_data_context, put_index_data, mydata);
}

static
netsnmp_variable_list*
get_next_data_point (
	void**			my_loop_context,
	void**			my_data_context,
	netsnmp_variable_list*	put_index_data,
	netsnmp_iterator_info*	mydata
	)
{
	assert (nullptr != mydata);
	auto table = static_cast<adh::snmp::table_t*> (mydata->myvoid);
	assert (nullptr != table);

	return table->getNextDataPoint (my_loop_context, my_data_context, put_index_data, mydata);
}

static
void
free_loop_context (
	void*			my_loop_context,
	netsnmp_iterator_info*	mydata
	)
{
	assert (nullptr != mydata);
	auto table = static_cast<adh::snmp::table_t*> (mydata->myvoid);
	assert (nullptr != table);

	return table->freeLoopContext (my_loop_context, mydata);
}

} /* anonymous namespace */

adh::snmp::table_t::table_t (
	const std::vector<oid>& oid,
	const std::string& name,
	const std::string& class_filter
	) :
	oid_ (oid),
	table_name_ (name),
	class_filter_ (class_filter),
	classId_ (class_filter.c_str())
{
}

/* Add SNMP row to table.  RMC object refers to a valid root server from the
 * managed application, which it is does not matter we ony require the instanceId.
 */
bool
adh::snmp::table_t::addRow (
	RTRProxyManagedObjectServer& server,
	const RTRProxyManagedObjectHandle& handle
	)
{
	auto row = std::make_shared<row_t> (server, handle);
	rows_.push_back (row);

/* add variables to row and prepare for object sync to attach additional interest */
	for (auto object : objects_) {
		if ((bool)object) {
LOG(INFO) << "adding to object: " << object->instanceId();
			auto variable = row->addVariable (*this, object);
			auto rmc_object = server.object (handle);
			variable->addRootObject (rmc_object);
		}
	}

/* get RMC to fan out events to the row */
	server.addClient (*row.get());
	if (RTRTRUE == server.inSync())
		row->processObjectServerSync (server);

	return true;
}

/* terrible RMC does not permit extraction of a RTRProxyManagedObjectServer
 * reference from a RTRProxyManagedObjectPtr event though it is sitting
 * inside the protected area.
 */
bool
adh::snmp::table_t::addRow (
	RTRProxyManagedObjectPtr rmc_object
	)
{
	auto row = std::make_shared<row_t> (rmc_object);
	rows_.push_back (row);

	for (auto object : objects_) {
		if ((bool)object) {
LOG(INFO) << "adding to object: " << object->instanceId();
			auto variable = row->addVariable (*this, object);
			variable->addClassObject (rmc_object);
		}
	}

/* no event propagation */
	return true;
}

/* Add SNMP object to table.
 */
bool
adh::snmp::table_t::addObject (
	oid oid,
	const RTRObjectId& instanceId,
	int object_attribute,
	const std::string& name,
	bool is_index,
	u_char asn_type
	)
{
	DCHECK (oid > 0);

/* validate provided oid with object ordering. */
	auto object = std::make_shared<object_t> (instanceId, object_attribute, name, asn_type, is_index);
	assert ((bool)object);
	if (objects_.size() < oid)
		objects_.resize (oid);
	objects_.at (oid - 1) = object;
	return true;
}

/* Register new SNMP table.  rows_ must be populated for SNMP indexing.
 */
int
adh::snmp::table_t::registerSnmpTable (void)
{
	DCHECK (!table_name_.empty());
	DCHECK (!oid_.empty());
	DCHECK (!objects_.empty());

	LOG(INFO) << "table: " << table_name_;

	netsnmp_handler_registration* reg = nullptr;
	netsnmp_iterator_info* iinfo = nullptr;
	netsnmp_table_registration_info* table_info = nullptr;

	reg = netsnmp_create_handler_registration (
		table_name_.c_str(),	handler,
		oid_.data(),		oid_.size(),
		HANDLER_CAN_RONLY
		);
	if (nullptr == reg)
		goto error;

	table_info = SNMP_MALLOC_TYPEDEF (netsnmp_table_registration_info);
	if (nullptr == table_info)
		goto error;
	if (!addIndexes (table_info))
		goto error;
    
	iinfo = SNMP_MALLOC_TYPEDEF (netsnmp_iterator_info);
	if (nullptr == iinfo)
		goto error;
	iinfo->get_first_data_point	= get_first_data_point;
	iinfo->get_next_data_point	= get_next_data_point;
	iinfo->free_loop_context_at_end	= free_loop_context;
	iinfo->myvoid			= (void*)this;
	iinfo->table_reginfo		= table_info;
   
	return netsnmp_register_table_iterator (reg, iinfo);

error:
	LOG(ERROR) << "MIB registration failed.";
	if (table_info && table_info->indexes)		/* table_data_free_func() is internal */
		snmp_free_var (table_info->indexes);
	SNMP_FREE (table_info);
	SNMP_FREE (iinfo);
	netsnmp_handler_registration_free (reg);
	return MIB_REGISTRATION_FAILED;
}

/* Add SNMP table indexes per configuration.
 */
bool
adh::snmp::table_t::addIndexes (
	netsnmp_table_registration_info* table_info
	)
{
	for (auto it = objects_.begin();
		it != objects_.end();
		++it)
	{
		auto& object = *it;
		if (!(bool)object || !object->isIndex())
			continue;
		netsnmp_table_helper_add_index (table_info, object->type());
	}
	table_info->min_column = 1;
	table_info->max_column = objects_.size();
	return true;
}

/* Set SNMP table row index values.
 */
bool
adh::snmp::table_t::setIndexedValues (
	netsnmp_variable_list* idx,
	std::shared_ptr<adh::snmp::row_t> row
	)
{
	unsigned idx_count = 0;
	for (auto it = row->variables_.begin();
		it != row->variables_.end();
		++it)
	{
		auto& variable = *it;
		if (!variable->isIndex())
			continue;
		if (!(bool)variable) {
LOG(INFO) << "index is not inSync";
			return false;
		}
		snmp_set_var_typed_value (idx, variable->type(), variable->data(), variable->size());
		idx = idx->next_variable;
		++idx_count;
	}
	return (idx_count > 0);
}

/* Create context for SNMP query.
 */
netsnmp_variable_list*
adh::snmp::table_t::getFirstDataPoint (
	void**			my_loop_context,	/* valid through one query of multiple "data points" */
	void**			my_data_context,	/* answer blob which is passed to handler() */
	netsnmp_variable_list*	put_index_data,		/* answer */
	netsnmp_iterator_info*	mydata			/* iinfo on init() */
	)
{
	LOG(INFO) << "getFirstDataPoint:" << table_name_;
	assert (nullptr != my_loop_context);
	assert (nullptr != my_data_context);
	assert (nullptr != put_index_data);
	assert (nullptr != mydata);

/* Create our own context for this SNMP loop, lock on list follows lifetime of context */
	auto context = std::make_shared<context_t> (*this);
	if (!(bool)context || context->empty())
		return nullptr;

/* Save context with NET-SNMP iterator. */
	*my_loop_context = context.get();
	context_t::global_list.push_back (std::move (context));

/* pass on for generic row access */
	return getNextDataPoint (my_loop_context, my_data_context, put_index_data, mydata);
}

/* Retrieve next SNMP query context, i.e. next row in table.
 */
netsnmp_variable_list*
adh::snmp::table_t::getNextDataPoint (
	void**			my_loop_context,
	void**			my_data_context,
	netsnmp_variable_list*	put_index_data,
	netsnmp_iterator_info*	mydata
	)
{
	LOG(INFO) << "getNextDataPoint:" << table_name_;
	assert (nullptr != my_loop_context);
	assert (nullptr != my_data_context);
	assert (nullptr != put_index_data);
	assert (nullptr != mydata);

	auto context = static_cast<context_t*>(*my_loop_context);
	netsnmp_variable_list *idx = put_index_data;
	std::shared_ptr<row_t> row;
	RTRProxyManagedVariablePtr instance;

	do {
/* end of data points */
		if (context->it == context->end) {
			LOG(INFO) << "end of data points";
			return nullptr;
		}

/* this plugin instance as a data point */
		row = *(context->it)++;

	} while (!(bool)row);

LOG(INFO) << "iteration, row=" << row->instance();

	if (!setIndexedValues (idx, row))
		return nullptr;

/* reference remains in list */
        *my_data_context = (void*)row.get();
        return put_index_data;
}

#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 5)
#else
class compare_context_functor {
public:
	compare_context_functor (adh::snmp::context_t* context) :
		context_ (context)
	{
	}

	bool operator() (const std::shared_ptr<adh::snmp::context_t>& shared_context) const
	{
		return shared_context.get() == context_;
	}

protected:
	adh::snmp::context_t* context_;
};
#endif

void
adh::snmp::table_t::freeLoopContext (
	void*			my_loop_context,
	netsnmp_iterator_info*	mydata
	)
{
	LOG(INFO) << "freeLoopContext:" << table_name_;
	assert (nullptr != my_loop_context);
	assert (nullptr != mydata);

/* delete context and shared lock on global list of all stitch objects */
	context_t* context = static_cast<context_t*>(my_loop_context);
/* I'm sure there must be a better method :-( */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 5)
	context_t::global_list.erase (std::remove_if (context_t::global_list.begin(),
		context_t::global_list.end(),
		[context](std::shared_ptr<context_t> shared_context) -> bool {
			return shared_context.get() == context;
	}));
#else
	context_t::global_list.erase (std::remove_if (context_t::global_list.begin(),
		context_t::global_list.end(),
		compare_context_functor (context)));
#endif
}

/* handles requests for the stitchPluginTable table
 */
int
adh::snmp::table_t::processSnmpQuery (
	netsnmp_mib_handler*		handler,
	netsnmp_handler_registration*	reginfo,
	netsnmp_agent_request_info*	reqinfo,
	netsnmp_request_info*		requests
	)
{
	LOG(INFO) << "processSnmpQuery:" << table_name_;
	assert (nullptr != handler);
	assert (nullptr != reginfo);
	assert (nullptr != reqinfo);
	assert (nullptr != requests);

	switch (reqinfo->mode) {

/* Read-support (also covers GetNext requests) */

	case MODE_GET:
	{
		for (netsnmp_request_info* request = requests;
		     request;
		     request = request->next)
		{
			const row_t* row = static_cast<row_t*>(netsnmp_extract_iterator_context (request));
			if (nullptr == row) {
				LOG(INFO) << "nullptr row";
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHINSTANCE);
				continue;
			}

			netsnmp_variable_list* var = request->requestvb;
			netsnmp_table_request_info* table_info = netsnmp_extract_table_info (request);
			if (nullptr == table_info) {
				snmp_log (__netsnmp_LOG_ERR, "handler: empty table request info.\n");
				continue;
			}
 
			auto variable = row->variables_[table_info->colnum - 1];
			if (!(bool)variable) {
				LOG(INFO) << "nullptr variable";
				snmp_log (__netsnmp_LOG_ERR, "handler: unknown column.\n");
				netsnmp_set_request_error (reqinfo, request, SNMP_NOSUCHOBJECT);
				break;
			}

			LOG(INFO) << "setting var";
			snmp_set_var_typed_value (var, variable->type(), variable->data(), variable->size());
			LOG(INFO) << "var set";
		}
		break;
	}

	default:
		snmp_log (__netsnmp_LOG_ERR, "adhTable_handler: unnsupported mode.\n");
		break;
	}
	return SNMP_ERR_NOERROR;
}

#if 0
void
adh::snmp::table_t::processObjectServerRootAdded (
	RTRProxyManagedObjectServer& server,
	const RTRProxyManagedObjectHandle& handle
	)
{
	LOG(INFO) << "ObjectServerRootAdded";

// fan out to all objects.
	auto root = server.object (handle);

	LOG(INFO) << "Propagating server root ...";
	for (auto it = objects_.begin();
		it != objects_.end();
		++it)
	{
		auto& object = *it;

		if (!(bool)object)
			continue;

		const auto& id = object->id();

LOG(INFO) << "id: " << id.string();
LOG(INFO) << "root: " << root->instanceId().string();
		if (!id.isDescendant (root->instanceId()))
			continue;
LOG(INFO) << id << " is descendant of " << root->instanceId();

		if (!(root->instanceId() == id.string()))
			continue;

		LOG(INFO) << "match name: " 
			<< object->id() << "."
			<< object->name();

// test random access
	RTRString o1 ("adh.serviceGenerator"),
		v1 ("SSLServiceGenerator");
	RTRObjectId iid1 (root->instanceId().parent(), o1);
	RTRProxyManagedObjectHandle pmoh ("", iid1, "");
	RTRProxyManagedObjectPtr pmo = server.object (pmoh);
	RTRProxyManagedNumericPtr numeric = pmo->numericByName (v1);
	LOG(INFO) << "numeric: " << numeric;
	}
}
#endif

// Event processing for RTRProxyManagedObjectServerPoolClient
void
adh::snmp::table_t::processProxyManagedObjectServerAdded (
        RTRProxyManagedObjectServerPool& pool,
        RTRProxyManagedObjectServer& server
        )
{
        LOG(INFO) << "ObjectServerAdded:" << table_name_;

// subscribe to serverSync, rootAdded, & rootRemoved events.
        server.addClient (*this);

        if (RTRTRUE == server.inSync())
                processObjectServerSync (server);
}

void
adh::snmp::table_t::processProxyManagedObjectServerRemoved (
        RTRProxyManagedObjectServerPool& pool,
        RTRProxyManagedObjectServer& server
        )
{
        LOG(INFO) << "ObjectServerRemoved:" << table_name_;

        server.dropClient (*this);
}

// Event processing -- RTRProxyManagedObjectServerClient
void
adh::snmp::table_t::processObjectServerError (
        RTRProxyManagedObjectServer& server
        )
{
        LOG(INFO) << "ObjectServerError:" << table_name_;

        server.dropClient (*this);
}

void
adh::snmp::table_t::processObjectServerSync (
        RTRProxyManagedObjectServer& server
        )
{
        LOG(INFO) << "ObjectServerSync:" << table_name_;

        auto it = server.roots();
        for (it.start(); !it.off(); it.forth())
                processObjectServerRootAdded (server, it.item());
}

void
adh::snmp::table_t::processObjectServerRootAdded (
        RTRProxyManagedObjectServer& server,
        const RTRProxyManagedObjectHandle& handle
        )
{
        LOG(INFO) << "ObjectServerRootAdded:" << table_name_;

        auto rmc_object = server.object (handle);

LOG(INFO) << "root object: " << rmc_object->instanceId().string();

	if (!hasClassFilter())
	{
/* extract instance from root object server:
 *      RTRApplicationId appId3("instance1", "applicationName");
 */
	        assert (rmc_object->instanceId().numberOfElements() >= 2);
		auto instance = rmc_object->instanceId().iTh (2);
		auto name = rmc_object->instanceId().name();
		RTRApplicationId application (instance, name);
		LOG(INFO) << "discovered application: " << application.string();

/* first root of this application? */
		const auto it = root_map_.find (application);
		root_map_.insert (std::make_pair (application, handle));
		if (it == root_map_.end())
			addRow (server, handle);
	}
	else
	{
/* root matches class */
		if (rmc_object->classId().equivalent (classId()))
			addRow (server, handle);

/* walk entire tree */
		push_back (rmc_object);
		rmc_object->addClient (*this);
		if (RTRTRUE == rmc_object->inSync())
			processProxyManagedObjectSync (*rmc_object);
	}
}

void
adh::snmp::table_t::processObjectServerRootRemoved (
        RTRProxyManagedObjectServer& server,
        const RTRProxyManagedObjectHandle& handle
        )
{
        LOG(INFO) << "ObjectServerRootRemoved:" << table_name_;

        auto object = server.object (handle);
        assert (object->instanceId().numberOfElements() >= 2);
        auto instance = object->instanceId().iTh (2);
        auto name = object->instanceId().name();
        RTRApplicationId application (instance, name);

        auto it = root_map_.find (application);
        root_map_.erase (it);
        it = root_map_.find (application);
        if (it == root_map_.end()) {
                LOG(INFO) << "application disconnected: " << application.string();
//              client->processManagedApplicationRemoved (application);
        }
}

// Event processing for RTRProxyManagedObjectClient
void
adh::snmp::table_t::processProxyManagedObjectError(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectError:" << table_name_ << ": " << object.text();
}

void
adh::snmp::table_t::processProxyManagedObjectSync(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectSync:" << table_name_;

LOG(INFO) << "sync for " << object.instanceId().string();

	auto it = object.childHandles();
	for (it.start(); !it.off(); it.forth())
	{
		auto child = object.childByName (it.item().name());
		if (child->error())
			break;
LOG(INFO) << "child: " << child->instanceId().string();

		if (child->classId().equivalent (classId()))
			addRow (child);
		push_back (child);
		child->addClient (*this);
		if (RTRTRUE == child->inSync())
			processProxyManagedObjectSync (*child);
	}
}

void
adh::snmp::table_t::processProxyManagedObjectDeleted(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectDeleted:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectInfo(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectInfo:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectInService(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectInService:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectRecovering(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectRecovering:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectWaiting(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectWaiting:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectDead(
        const RTRProxyManagedObject& object
        )
{
        LOG(INFO) << "ObjectDead:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectChildAdded(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedObjectHandle& handle
        )
{
        LOG(INFO) << "ObjectChildAdded:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectChildRemoved(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedObjectHandle& handle
        )
{
        LOG(INFO) << "ObjectChildRemoved:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectVariableAdded(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedVariableHandle& handle
        )
{
        LOG(INFO) << "ObjectVariableAdded:" << table_name_;
}

void
adh::snmp::table_t::processProxyManagedObjectVariableRemoved(
        const RTRProxyManagedObject& object,
        const RTRProxyManagedVariableHandle& handle
        )
{
        LOG(INFO) << "ObjectVariableRemoved:" << table_name_;
}

void
adh::snmp::table_t::push_back (
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
