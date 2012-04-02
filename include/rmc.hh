/* The missing header rmc.hh
 */

#ifndef __RMC_MISSING_HH__
#define __RMC_MISSING_HH__

#ifndef x86_Linux_4X
#	define x86_Linux_4X
#	define RMC_x86_Linux_4X_DEFINED
#endif
#ifndef x86_Linux_5X
#	define x86_Linux_5X
#	define RMC_x86_Linux_5X_DEFINED
#endif
#ifndef x86_Linux_6X
#	define x86_Linux_6X
#	define RMC_x86_Linux_6X_DEFINED
#endif
#ifndef _iso_stdcpp_
#	define _iso_stdcpp_
#	define RMC__iso_stdcpp__DEFINED
#endif
#ifndef x86_Linux_CLOCK_MONOTONIC
#	define x86_Linux_CLOCK_MONOTONIC
#	define RMC_x86_Linux_CLOCK_MONOTONIC_DEFINED
#endif
#ifndef LinuxVersion
#	define LinuxVersion 6
#	define RMC_LinuxVersion_DEFINED
#endif
#ifndef COMPILE_64BITS
#	define COMPILE_64BITS
#	define RMC_COMPILE_64BITS_DEFINED
#endif
#ifndef _REENTRANT
#	define _REENTRANT
#	define RMC__REENTRANT_DEFINED
#endif
#ifndef _SVID_SOURCE
#	define _SVID_SOURCE 1
#	define RMC__SVID_SOURCE_DEFINED
#endif
#ifndef _BSD_SOURCE
#	define _BSD_SOURCE 1
#	define RMC__BSD_SOURCE_DEFINED
#endif
#ifndef _POSIX_SOURCE
#	define _POSIX_SOURCE 1
#	define RMC__POSIX_SOURCE_DEFINED
#endif
#ifndef _POSIX_C_SOURCE
#	define _POSIX_C_SOURCE 199506L
#	define RMC__POSIX_C_SOURCE_DEFINED
#endif
#ifndef _XOPEN_SOURCE
#	define _XOPEN_SOURCE 500
#	define RMC__XOPEN_SOURCE_DEFINED
#endif
#ifndef Linux
#	define Linux
#	define RMC_Linux_DEFINED
#endif
#ifndef LINUX
#	define LINUX
#	define RMC_LINUX_DEFINED
#endif
#ifndef DEV_LITTLE_ENDIAN
#	define DEV_LITTLE_ENDIAN
#	define RMC_DEV_LITTLE_ENDIAN_DEFINED
#endif
#ifndef DEV_POS_THR
#	define DEV_POS_THR
#	define RMC_DEV_POS_THR_DEFINED
#endif
#ifndef _POSIX_PTHREAD_SEMANTICS
#	define _POSIX_PTHREAD_SEMANTICS
#	define RMC__POSIX_PTHREAD_SEMANTICS_DEFINED
#endif
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#	define RMC__GNU_SOURCE_DEFINED
#endif
#ifndef TRACING_ENABLED
#	define TRACING_ENABLED
#	define RMC_TRACING_ENABLED_DEFINED
#endif
#ifndef _RDEV_NO_STL_
#	define _RDEV_NO_STL_
#	define RMC__RDEV_NO_STL__DEFINED
#endif

#include <rtr/selectni.h>               // Event loop implementation
#include <rtr/shmpmosp.h>               // RTRShmProxyManagedObjectServerPool

#include <rtr/pmosp.h>          // RTRProxyManagedObjectServerPool
#include <rtr/pmospc.h>         // RTRProxyManagedObjectServerPoolClient
#include <rtr/prxymos.h>        // RTRProxyManagedObjectServer
#include <rtr/pmosc.h>          // RTRProxyManagedObjectServerClient
#include <rtr/proxymo.h>        // RTRProxyManagedObject and all Variables
#include <rtr/timercmd.h>       // RTRTimerCmd

#include <rtr/appid.h>		// RTRApplicationId
#include <rtr/pmopc.h>		// RTRProxyManagedObjectPoolClient
#include <rtr/shmpmos.h>	// RTRShmProxyManagedObjectServer
#include <rtr/shmpmocdf.h>      // RTRShmProxyManagedObjectClassDirFactory
#include <rtr/pmocdc.h>		// RTRProxyManagedObjectClassDirectoryClient
#include <rtr/rtrnotif.h>

#ifdef RMC_x86_Linux_4X_DEFINED
#	undef x86_Linux_4X
#endif
#ifdef RMC_x86_Linux_5X_DEFINED
#	undef x86_Linux_5X
#endif
#ifdef RMC_x86_Linux_6X_DEFINED
#	undef x86_Linux_6X
#endif
#ifdef RMC__iso_stdcpp__DEFINED
#	undef _iso_stdcpp_
#endif
#ifdef RMC_x86_Linux_CLOCK_MONOTONIC_DEFINED
#	undef x86_Linux_CLOCK_MONOTONIC
#endif
#ifdef RMC_LinuxVersion_DEFINED
#	undef LinuxVersion 
#endif
#ifdef RMC_COMPILE_64BITS_DEFINED
#	undef COMPILE_64BITS
#endif
#ifdef RMC__REENTRANT_DEFINED
#	undef _REENTRANT
#endif
#ifdef RMC__SVID_SOURCE_DEFINED
#	undef _SVID_SOURCE
#endif
#ifdef RMC__BSD_SOURCE_DEFINED
#	undef _BSD_SOURCE
#endif
#ifdef RMC__POSIX_SOURCE_DEFINED
#	undef _POSIX_SOURCE
#endif
#ifdef RMC__POSIX_C_SOURCE_DEFINED
#	undef _POSIX_C_SOURCE
#endif
#ifdef RMC__XOPEN_SOURCE_DEFINED
#	undef _XOPEN_SOURCE
#endif
#ifdef RMC_Linux_DEFINED
#	undef Linux
#endif
#ifdef RMC_LINUX_DEFINED
#	undef LINUX
#endif
#ifdef RMC_DEV_LITTLE_ENDIAN_DEFINED
#	undef DEV_LITTLE_ENDIAN
#endif
#ifdef RMC_DEV_POS_THR_DEFINED
#	undef DEV_POS_THR
#endif
#ifdef RMC__POSIX_PTHREAD_SEMANTICS_DEFINED
#	undef _POSIX_PTHREAD_SEMANTICS
#endif
#ifdef RMC__GNU_SOURCE_DEFINED
#	undef _GNU_SOURCE
#endif
#ifdef RMC_TRACING_ENABLED_DEFINED
#	undef TRACING_ENABLED
#endif
#ifdef RMC__RDEV_NO_STL__DEFINED
#	undef _RDEV_NO_STL_
#endif

#endif /* __RMC_MISSING_HH__ */

/* eof */
