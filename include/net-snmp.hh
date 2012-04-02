/* The missing header net-snmp.hh
 */

#ifndef __NETSNMP_MISSING_HH__
#define __NETSNMP_MISSING_HH__

/* redirect namespace pollution */
#define U64 __netsnmp_U64

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* revert Net-snmp namespace pollution */
#undef U64

/* non-aggregate types for tree end nodes */
#undef TYPE_NULL
#undef TYPE_INTEGER

#define __netsnmp_LOG_EMERG     0
#define __netsnmp_LOG_ALERT     1
#define __netsnmp_LOG_CRIT      2
#define __netsnmp_LOG_ERR       3
#define __netsnmp_LOG_WARNING   4
#define __netsnmp_LOG_NOTICE    5
#define __netsnmp_LOG_INFO      6
#define __netsnmp_LOG_DEBUG     7
static_assert (__netsnmp_LOG_EMERG   == LOG_EMERG,   "LOG_EMERGE mismatch");
static_assert (__netsnmp_LOG_ALERT   == LOG_ALERT,   "LOG_ALERT mismatch");
static_assert (__netsnmp_LOG_CRIT    == LOG_CRIT,    "LOG_CRIT mismatch");
static_assert (__netsnmp_LOG_ERR     == LOG_ERR,     "LOG_ERR mismatch");
static_assert (__netsnmp_LOG_WARNING == LOG_WARNING, "LOG_WARNING mismatch");
static_assert (__netsnmp_LOG_NOTICE  == LOG_NOTICE,  "LOG_NOTICE mismatch");
static_assert (__netsnmp_LOG_INFO    == LOG_INFO,    "LOG_INFO mismatch");
static_assert (__netsnmp_LOG_DEBUG   == LOG_DEBUG,   "LOG_DEBUG mismatch");
#undef LOG_EMERG
#undef LOG_ALERT
#undef LOG_CRIT
#undef LOG_ERR
#undef LOG_WARNING
#undef LOG_NOTICE
#undef LOG_INFO
#undef LOG_DEBUG

#endif /* __NETSNMP_MISSING_HH__ */

/* eof */
