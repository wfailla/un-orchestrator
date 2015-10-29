#include "ivshmem_cmdline_generator.h"

#define DPDKR_FORMAT "dpdkr%d"
#define DPDKR_TX_FORMAT DPDKR_FORMAT"_tx"
#define DPDKR_RX_FORMAT DPDKR_FORMAT"_rx"

bool IvshmemCmdLineGenerator::init = false;

IvshmemCmdLineGenerator::IvshmemCmdLineGenerator()
{
	//TODO: handle error in initializetion
	dpdk_init();
}


bool IvshmemCmdLineGenerator::dpdk_init(void)
{
	if(init)
		return true;

	/* does not exist a nicer way to do it? */
	char arg0[] = "./something";
	char arg1[] = "--proc-type=secondary";
	char arg2[] = "-c";
	char arg3[] = "0x1";
	char arg4[] = "-n";
	char arg5[] = "4";
	char arg6[] = "--";

	char * argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};
	int argc = 7;

	if(rte_eal_init(argc, argv) < 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DPDK can not be initialized");
		return false;
	}

	init = true;
	return true;
}

bool IvshmemCmdLineGenerator::get_cmdline(const char * port_name, char * cmdline, int size)
{
	char ring_name[20];
	int port_no;
	struct rte_ring * rx;
	struct rte_ring * tx;
	struct rte_mempool * packets_pool;
	
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Generating command line for port '%s'", port_name);

	/*lazy dpdk initialization */
	if(!init)
	{
		if(dpdk_init() < 0)
			return false;
	}

	/* it has to read just one integer that is the port name */
	if(sscanf(port_name, DPDKR_FORMAT, &port_no) != 1)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Port (%s) has bad port name format", port_name);
		return true;
	}

	/* look for the transmission ring */
	snprintf(ring_name, 20, DPDKR_TX_FORMAT, port_no);
	tx = rte_ring_lookup(ring_name);
	if(tx == NULL)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "port (%s) can not be found", port_name);
		return false;
	}

	/* look fot the reception ring */
	snprintf(ring_name, 20, DPDKR_RX_FORMAT, port_no);
	rx = rte_ring_lookup(ring_name);
	if(rx == NULL)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "port (%s) can not be found", port_name);
		return false;
	}

	/* look for the memory pool */
	/*
	* XXX: improve the wasy the memory pool is looked, the name could not
	* always be the same
	*/
	packets_pool = rte_mempool_lookup("ovs_mp_1500_0_262144");
	if(packets_pool == NULL)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "OVS packets mempool can not be found");
		return false;
	}

	if (rte_ivshmem_metadata_create(port_name) < 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Metadata file can not be created");
		return false;
	}

	if(rte_ivshmem_metadata_add_ring(tx, port_name) < 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Port (%s) can not be added to metadata_file", port_name);
		return false;
	}

	if(rte_ivshmem_metadata_add_ring(rx, port_name) < 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Port (%s) can not be added to metadata_file", port_name);
		return false;
	}

	if(rte_ivshmem_metadata_add_mempool(packets_pool, port_name) < 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "OVS packets mmepool can not be added to metadatafile");
		return false;
	}

	if (rte_ivshmem_metadata_cmdline_generate(cmdline, size, port_name) < 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Command line can not be generated", port_name);
		return false;
	}
	
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Command line: '%s'", cmdline);

	return true;
}
