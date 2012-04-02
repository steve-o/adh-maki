/* main entry point.
 */

#include "adh_maki.hh"

#include <cstdlib>

#include "chromium/command_line.hh"
#include "chromium/logging.hh"
#include "nullptr.hh"

static
bool
log_handler (
	int			severity,
	const char*		file,
	int			line,
	size_t			message_start,
	const std::string&	str
	)
{
	fprintf (stdout, "%s", str.c_str());
	fflush (stdout);
	return true;
}

int
main (
	int		argc,
	const char*	argv[]
	)
{
#ifdef _MSC_VER
/* Suppress abort message. */
	_set_abort_behavior (0, ~0);
#endif

	CommandLine::Init (argc, argv);
	logging::InitLogging(
		nullptr,
		logging::LOG_NONE,
		logging::DONT_LOCK_LOG_FILE,
		logging::APPEND_TO_OLD_LOG_FILE,
		logging::ENABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS
		);
	logging::SetLogMessageHandler (log_handler);

	adh::maki_t maki;
	return maki.run();
}

/* eof */
