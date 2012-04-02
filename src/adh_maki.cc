/* ADH maki.
 */

#include "adh_maki.hh"

#include "chromium/command_line.hh"
#include "chromium/logging.hh"

#include "adh_agent.hh"

static const char* kMaki ("maki");
static const char* kMakiInstance ("1");
static const char* kManagedApplication ("adh");
/* monitor all instances */

static const char* kPoolName ("pool");

const char kUsageText[] = 
	"rmcwalk [--shmkey=<key>] [-X] [-x=<address>]\n"
	"  --shmkey=<key>\n"
	"Attaches to the shared memory segment created with shmkey.\n"
	"  -X\n"
	"Run as an AgentX subagent rather than as an SNMP master agent.\n"
	"  -x=<address>\n"
	"Connects to SNMP master agent on the specified address rather the default\n"
	"\"/var/agentx/master\".  The address can either be a Unix domain socket path,\n"
	"or the address of a network interface.\n";

namespace switches {
const char kHelp[]		= "help";
const char kSharedMemoryKey[]	= "shmkey";
const char kAgentXSubagent[]	= "X";
const char kAgentXAddress[]	= "x";
} /* namespace switches */

const char* kDefaultSharedMemoryKey = "80";

static __attribute__ ((noreturn))
void
usage (
	const char* err
	)
{
	fprintf (stderr, "%s\n", err);
	exit (129);
}

static
void
on_signal (
	int signum
	)
{
	eventNotifierInit.notifier->disable();
}

adh::maki_t::maki_t()
{
}

adh::maki_t::~maki_t()
{
	clear();
	LOG(INFO) << "fin.";
}

int
adh::maki_t::run()
{
	maki_ = std::make_shared<RTRApplicationId> (kMakiInstance, kMaki);
	if (!(bool)maki_)
		return EXIT_FAILURE;

	context_ = std::make_shared<RTRObjectId> (*maki_.get(), kManagedApplication);
	if (!(bool)context_)
		return EXIT_FAILURE;

	pool_ = std::make_shared<RTRShmProxyManagedObjectServerPool> (*context_.get(), kPoolName);
	if (!(bool)pool_)
		return EXIT_FAILURE;

	CommandLine* command_line = CommandLine::ForCurrentProcess();

	if (command_line->HasSwitch (switches::kHelp)) {
		usage (kUsageText);
	}

/* shared memory key to connect to RMC managed application */
	std::string shmkey (command_line->GetSwitchValueASCII (switches::kSharedMemoryKey));
	if (shmkey.empty()) {
		shmkey = kDefaultSharedMemoryKey;
	}

/* AgentX subagent or master agent */
	bool is_agentx_subagent = false;
	if (command_line->HasSwitch (switches::kAgentXSubagent)) {
		is_agentx_subagent = true;
	}

/* AgentX socket address */
	std::string agentx_address (command_line->GetSwitchValueASCII (switches::kAgentXAddress));
	if (!agentx_address.empty()) {
		is_agentx_subagent = true;
	}

	agent_ = std::make_shared<agent_t> (is_agentx_subagent, agentx_address.c_str());
	if (!(bool)agent_)
		return EXIT_FAILURE;

	LOG(INFO) << "adding shared memory key " << shmkey;
	pool_->addServer (shmkey.c_str());

// capture terminate signals
	sighandler_t old_sighup_handler = signal (SIGHUP, SIG_IGN),
		old_sigint_handler = signal (SIGINT, on_signal),
		old_sigterm_handler = signal (SIGTERM, on_signal);

	LOG(INFO) << "Init complete, entering main loop.";
	mainLoop();
	LOG(INFO) << "Main loop terminated.";

// restore previous signals
	signal (SIGHUP, old_sighup_handler);
	signal (SIGINT, old_sigint_handler);
	signal (SIGTERM, old_sigterm_handler);	
	return EXIT_SUCCESS;
}

void
adh::maki_t::mainLoop()
{
/* SNMP main loop */
	agent_->run();

	LOG(INFO) << "monitoring server pool";
	agent_->addServerPool (pool_);

/* RMC main loop */
        RTRSelectNotifier::run();
}

void
adh::maki_t::clear()
{
	agent_.reset();
	pool_.reset();
	context_.reset();
	maki_.reset();
}


/* eof */
