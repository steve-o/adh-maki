/* SNMP table column, a.k.a. a SNMP object.
 */

#ifndef __SNMP_OBJECT_HH__
#define __SNMP_OBJECT_HH__
#pragma once

#include "snmp_variable.hh"

namespace adh {
namespace snmp {

	class object_t
	{
	public:
		object_t (const RTRObjectId& instanceId, int object_attribute, const std::string& name, u_char asn_type, bool is_index) :
			instanceId_ (instanceId),
			object_attribute_ (object_attribute),
			name_ (name),
			is_index_ (is_index),
			asn_type_ (asn_type)
		{
		}

/* observers */
		const RTRObjectId& instanceId()
		{
			return instanceId_;
		}

		int attribute()
		{
			return object_attribute_;
		}

		const std::string& name()
		{
			return name_;
		}

		u_char type()
		{
			return asn_type_;
		}

		bool isObjectVariable()
		{
			return (RMC_ATTRIBUTE_ACTIVE_VALUE == object_attribute_);
		}

		bool isIndex()
		{
			return is_index_;
		}

	protected:
/* RMC object identifier */
		RTRObjectId instanceId_;
/* RMC variable or attribute name */
		int object_attribute_;
		std::string name_;
/* whether this SNMP object is an index for the row */
		bool is_index_;
/* the ASN.1 simple type of this object */
		u_char asn_type_;
	};

} /* namespace snmp */
} /* namespace adh */

#endif /* __SNMP_OBJECT_HH__ */

/* eof */
