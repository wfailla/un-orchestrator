#include <microhttpd.h>
#include "rest_server.h"
#include "ivshmem_cmdline_generator.h"

#define DEFAULT_REST_PORT 6666
#define MODULE_NAME "cmdline-generator"

int main(int argc, char * argv[])
{
	struct MHD_Daemon * daemon;

	LOG(ORCH_INFO, MODULE_NAME, "Opening...");

	//Check for root privileges
	if(geteuid() != 0)
	{
		LOG(ORCH_ERROR, MODULE_NAME,  "%s: You must be sudo", argv[0]);
		exit(EXIT_FAILURE);
	}

	daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, DEFAULT_REST_PORT, NULL, NULL,
			&RestServer::answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED,
			&RestServer::request_completed, NULL,MHD_OPTION_END);

	if(daemon == NULL)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "http server can not be started");
		exit(EXIT_FAILURE);
	}

	getchar();

	MHD_stop_daemon (daemon);

	LOG(ORCH_INFO, MODULE_NAME, "Closing...");

	return 0;
}
