/* ADH-SNMP agent, single session.
 */

#include "adh_agent.hh"

#include "chromium/json/json_reader.hh"
#include "chromium/logging.hh"
#include "chromium/string_split.hh"
#include "chromium/values.hh"

#include "snmp_table.hh"
#include "snmp_variable.hh"

static const char* kApplicationName = "adh";
/* NET-SNMP will read adh-maki.conf */
static const char* kSnmpApplicationName = "adh-maki";

/* JSON configuration of this agent */
static const char kAdhJson[] = "{\"oid\":\"1.3.6.1.4.1.67.14.1.5\",\"tables\":[{\"oid\":1,\"name\":\"adhTable\",\"objects\":[{\"oid\":1,\"object\":\"adh\",\"attribute\":\"active value\",\"name\":\"instance\",\"type\":\"ASN_OCTET_STR\",\"index\":true},{\"oid\":2,\"object\":\"adh\",\"attribute\":\"object state\",\"type\":\"ASN_OCTET_STR\"},{\"oid\":3,\"object\":\"adh.sourceApplicationPool.incomingMessages\",\"attribute\":\"active value\",\"name\":\"updateRate\",\"type\":\"ASN_INTEGER\"}]},{\"oid\":93,\"name\":\"serverSrcDistTable\",\"class\":\"Server.SrcDist\",\"objects\":[{\"oid\":1,\"object\":\"\",\"attribute\":\"object name\",\"type\":\"ASN_OCTET_STR\",\"index\":true},{\"oid\":2,\"object\":\"outgoingMessages\",\"attribute\":\"active value\",\"name\":\"updateRate\",\"type\":\"ASN_INTEGER\"}]}]}";

adh::agent_t::agent_t (
	bool is_agentx_subagent,
	const char* agentx_socket
	) :
	super (kSnmpApplicationName, is_agentx_subagent, agentx_socket)
{
}

/* add RMC server pool from shared memory key
 */
bool
adh::agent_t::addServerPool (
	std::shared_ptr<RTRProxyManagedObjectServerPool> pool
	)
{
// subscribe to serverAdded & serverRemoved events.
	LOG(INFO) << "subscribing to RMC server pool events.";
	for (auto table : tables_) {
		pool->addClient (*table.get());
		auto it = pool->servers();
		for (it.start(); !it.off(); it.forth()) {
			auto& server = static_cast<RTRProxyManagedObjectServer&> (*it);
			table->processProxyManagedObjectServerAdded (*pool.get(), server);
		}
	}
	return true;
}

bool
adh::agent_t::init_MIB (void)
{
/* here we initialize all the tables we're planning on supporting */
	std::unique_ptr<chromium::Value> root;

	LOG(INFO) << "reading JSON ...";	
	root.reset (chromium::JSONReader::Read (kAdhJson, false));
	LOG(INFO) << "read.";

/* walk JSON tree and create SNMP tree */
	assert (root.get());
	assert (root->IsType (chromium::Value::TYPE_DICTIONARY));
	if (!createSnmpRoot (static_cast<chromium::DictionaryValue*>(root.get())))
		return false;

/* walk SNMP tree and register tables */
	LOG(INFO) << "Registering MIB tables ...";
	for (auto it = tables_.begin();
		it != tables_.end();
		++it)
	{
		auto& table = *it;
		if (MIB_REGISTRATION_FAILED == table->registerSnmpTable())
			return false;
	}

	LOG(INFO) << "MIB complete.";
	return true;
}

/* {
 * 	"oid": 1.2.3.4,
 * 	"objects": []
 * }
 */
bool
adh::agent_t::createSnmpRoot (
	chromium::DictionaryValue* dict_val
	)
{
	LOG(INFO) << "createSnmpRoot";

	std::string oid;
	CHECK (dict_val->GetString ("oid", &oid));
	std::vector<std::string> ids;
	chromium::SplitString (oid, '.', &ids);
	std::for_each (ids.begin(), ids.end(), [&](std::string& str){
		root_oid_.emplace_back (std::stoi (str));
	});

	LOG(INFO) << "root oid: " << oid;

	chromium::ListValue* list = nullptr;
	CHECK (dict_val->GetList ("tables", &list));
	for (unsigned i = 0; i < list->GetSize(); ++i) {
		chromium::Value* tmp_value;
		CHECK (list->Get (i, &tmp_value));
		assert (tmp_value->IsType (chromium::Value::TYPE_DICTIONARY));
		if (!createSnmpTable (static_cast<chromium::DictionaryValue*>(tmp_value)))
			return false;
	}

	LOG(INFO) << "root complete.";
	return true;
}

/* {
 * 	"oid": 1,
 * 	"name": "moo",
 * 	"class": "cows",
 * 	"objects": []
 * }
 */
bool
adh::agent_t::createSnmpTable(
	chromium::DictionaryValue* dict_val
	)
{
	LOG(INFO) << "createSnmpTable";

/* required */
	int integer_value = 0;
	CHECK (dict_val->GetInteger ("oid", &integer_value));
	std::vector<oid> oid (root_oid_);
	oid.push_back (integer_value);
	std::string name;
	CHECK (dict_val->GetString ("name", &name));
	chromium::ListValue* list = nullptr;
	CHECK (dict_val->GetList ("objects", &list));

/* optional */
	std::string class_filter;
	bool has_class_filter = dict_val->GetString ("class", &class_filter);

	LOG(INFO) << "oid: " << integer_value
		<< " name: " << name
		<< " class: " << class_filter;

	auto table = std::make_shared<snmp::table_t> (oid, name, class_filter);
	tables_.push_back (table);

/* objects */
	for (unsigned i = 0; i < list->GetSize(); ++i) {
		chromium::Value* tmp_value;
		CHECK (list->Get (i, &tmp_value));
		assert (tmp_value->IsType (chromium::Value::TYPE_DICTIONARY));
		if (!createSnmpObject (table, static_cast<chromium::DictionaryValue*>(tmp_value)))
			return false;
	}

	return true;
}

/* {
 * 	"oid": 1,
 * 	"object": "moo",
 * 	"attribute": "flavour",
 * 	"name": "milk",
 * 	"type": "ASN_OCTET_STR",
 * 	"index": true
 * }
 *
 * SNMP object equivalent to a column in a SNMP table.
 */
bool
adh::agent_t::createSnmpObject (
	std::shared_ptr<adh::snmp::table_t> table,
	chromium::DictionaryValue* dict_val
	)
{
	LOG(INFO) << "createSnmpObject";

/* required */
	int integer_value = 0;
	CHECK (dict_val->GetInteger ("oid", &integer_value));
	oid oid (integer_value);
	std::string object, attribute, type;
	CHECK (dict_val->GetString ("object", &object));
	CHECK (dict_val->GetString ("attribute", &attribute));
	CHECK (dict_val->GetString ("type", &type));

/* optional */
	std::string value;
	bool is_index = false;
	bool has_name = dict_val->GetString ("name", &value);
	bool has_is_index = dict_val->GetBoolean ("index", &is_index);

	LOG(INFO) << "oid: " << integer_value
		<< " object: " << object
		<< " attribute: \"" << attribute << '"'
		<< " type: " << type
		<< " name: " << value
		<< " is_index: " << is_index;

/* convert name to instance */
//	RTRApplicationId id (instance_.c_str(), object.c_str());
	RTRObjectId instanceId (object.c_str());
LOG(INFO) << "instanceId = " << instanceId;

/* convert attribute */
	int object_attribute = adh::snmp::RMC_ATTRIBUTE_INVALID;
	if ("active value" == attribute)	object_attribute = adh::snmp::RMC_ATTRIBUTE_ACTIVE_VALUE;
	else if ("object state" == attribute)	object_attribute = adh::snmp::RMC_ATTRIBUTE_OBJECT_STATE;
	else if ("object name" == attribute)	object_attribute = adh::snmp::RMC_ATTRIBUTE_OBJECT_NAME;
	else {
		NOTIMPLEMENTED();
	}
	assert (adh::snmp::RMC_ATTRIBUTE_INVALID != object_attribute);

/* convert ASN.1 type to native */
	u_char asn_type = 0;
	if ("ASN_OCTET_STR" == type)    asn_type = ASN_OCTET_STR;
	else if ("ASN_INTEGER" == type) asn_type = ASN_INTEGER;
	else if ("ASN_BOOLEAN" == type) asn_type = ASN_BOOLEAN;
	else {
		NOTIMPLEMENTED();
	}

	return table->addObject (oid, instanceId, object_attribute, value, is_index, asn_type);
}

/* eof */
