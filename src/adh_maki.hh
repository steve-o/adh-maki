/* ADH maki.
 */

#ifndef __ADH_MAKI_HH__
#define __ADH_MAKI_HH__
#pragma once

#include <memory>

/* Reuters Management Crap */
#include <rmc.hh>

/* Boost noncopyable base class */
#include <boost/utility.hpp>

namespace adh
{
	class agent_t;
	class monitor_t;

	class maki_t :
		boost::noncopyable
	{
	public:
		maki_t();
		~maki_t();

/* Run the provider with the given command-line parameters.
 * Returns the error code to be returned by main().
 */
		int run();
		void clear();

	protected:
		std::shared_ptr<RTRApplicationId> maki_;
		std::shared_ptr<RTRObjectId> context_;
		std::shared_ptr<RTRShmProxyManagedObjectServerPool> pool_;

		std::shared_ptr<RTRApplicationId> managed_;
		std::shared_ptr<agent_t> agent_;

/* Run core event loop. */
		void mainLoop();
	};

} /* namespace adh */

#endif /* __ADH_MAKI_HH__ */

/* eof */
