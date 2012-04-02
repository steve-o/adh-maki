# CMake build for adh-maki
#
# WARNING: Instability will ensue if mixed Boost versions are used, such as
# using the headers from a local build and the libraries from the system.

cmake_minimum_required (VERSION 2.6)

project (adh-maki)

#set(CMAKE_CXX_COMPILER "/home/steve-o/project/gcc-4.7.0/rtf/bin/c++")

set(RMC_ROOT /home/steve-o/project/rmc2.1.0.L1.linux.rrg)
set(RMC_INCLUDE_DIRS ${RMC_ROOT}/include)
set(RMC_LIBRARY_DIRS ${RMC_ROOT}/lib/RHEL6_64_GCC444)
set(RMC_LIBRARIES
	rmcsml_NO_STL
	rmccon_NO_STL
# note order is different to RMC example builds
	rmcpthrd_NO_STL
	rt
)

set(NETSNMP_ROOT /home/steve-o/project/netsnmp)
set(NETSNMP_INCLUDE_DIRS ${NETSNMP_ROOT}/include)
set(NETSNMP_LIRBARY_DIRS ${NETSNMP_ROOT}/lib)
set(NETSNMP_LIBRARIES
	netsnmp
	netsnmpagent
	netsnmpmibs
)

set(BOOST_ROOT /home/steve-o/project/gcc-4.7.0/rtf)
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib)
#set(BOOST_ROOT /home/steve-o/project/boost_1_48_0)
#set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

# Boost.Atomic outstanding from 1.49
set(BOOST_ATOMIC_INCLUDE_DIRS /home/steve-o/project/boost.atomic)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.44 COMPONENTS thread REQUIRED)
find_package(Threads REQUIRED)

#-----------------------------------------------------------------------------
# force off-tree build

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
message(FATAL_ERROR "CMake generation is not allowed within the source directory! 
Remove the CMakeCache.txt file and try again from another folder, e.g.: 

   del CMakeCache.txt 
   mkdir cmake-make 
   cd cmake-make
   cmake ..
")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

#-----------------------------------------------------------------------------
# default to Release build

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

#-----------------------------------------------------------------------------
# platform specifics

add_definitions(
# C++11
	-std=c++0x
)

#-----------------------------------------------------------------------------
# source files

set(gcc_sources
	src/chromium/atomicops_internals_x86_gcc.cc
)
set(posix_sources
	src/chromium/debug/stack_trace_posix.cc
	src/chromium/safe_strerror_posix.cc
	src/chromium/synchronization/lock_impl_posix.cc
)
set(win32_sources
	src/chromium/debug/stack_trace_win.cc
	src/chromium/synchronization/lock_impl_win.cc
)

set(cxx-sources
	src/adh_agent.cc
	src/adh_maki.cc
	src/main.cc
	src/snmp_agent.cc
	src/snmp_row.cc
	src/snmp_table.cc
	src/snmp_variable.cc
	src/chromium/chromium_switches.cc
	src/chromium/command_line.cc
	src/chromium/debug/stack_trace.cc
	src/chromium/json/json_reader.cc
	src/chromium/memory/singleton.cc
#	src/chromium/metrics/histogram.cc
#	src/chromium/metrics/stats_counters.cc
#	src/chromium/metrics/stats_table.cc
	src/chromium/logging.cc
	src/chromium/string_number_conversions.cc
	src/chromium/string_piece.cc
	src/chromium/stringprintf.cc
	src/chromium/string_split.cc
	src/chromium/string_util.cc
	src/chromium/synchronization/lock.cc
	src/chromium/values.cc
	src/chromium/vlog.cc
	${gcc_sources}
	${posix_sources}
)

include_directories(
	include
	${Boost_INCLUDE_DIRS}
	${BOOST_ATOMIC_INCLUDE_DIRS}
	${NETSNMP_ROOT}/include
	${RMC_INCLUDE_DIRS}
)

link_directories(
# FindBoost.cmake incorrectly defaults to the system copy instead of
# BOOST_ROOT, as a workaround force directory.
#	${Boost_LIBRARY_DIRS}
	${BOOST_LIBRARYDIR}
	${NETSNMP_ROOT}/lib
	${RMC_LIBRARY_DIRS}
)

#-----------------------------------------------------------------------------
# output

add_executable(adh-maki ${cxx-sources})

target_link_libraries(adh-maki
	${RMC_LIBRARIES}
	${NETSNMP_LIBRARIES}
#	${Boost_LIBRARIES}
# explicit name is required to bypass dynamic linking to system copy
	${Boost_LIB_PREFIX}boost_thread${CMAKE_STATIC_LIBRARY_SUFFIX}
# manually add threading dependencies
	${CMAKE_THREAD_LIBS_INIT}
)

# end of file
