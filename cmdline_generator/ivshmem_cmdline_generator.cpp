#include <stdio.h>
#include <string.h>

#include <rte_config.h>
#include <rte_eal.h>
#include <rte_ivshmem.h>
#include <rte_mempool.h>
#include <rte_ring.h>

#include "logger.h"

#define DPDKR_FORMAT "dpdkr%d"
#define DPDKR_TX_FORMAT DPDKR_FORMAT"_tx"
#define DPDKR_RX_FORMAT DPDKR_FORMAT"_rx"

#define MODULE_NAME "cmdline-generator"

static int init;

/*
 * initializes dpdk as a secondary process using fake arguments
 */

static int
dpdk_init(void)
{
	int retval;

	if(init)
		return 0;

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
		LOG(ORCH_ERROR, MODULE_NAME, "DPDK can not be initialized");
		return -1;
	}

	init = 1;
	return 0;
}

int get_cmdline(const char * port_name, char * cmdline, int size)
{
	char ring_name[20];
	int port_no;
	struct rte_ring * rx;
	struct rte_ring * tx;
	struct rte_mempool * packets_pool;

	/*lazy dpdk initialization */
	if(!init)
	{
		if(dpdk_init() < 0)
			return -1;
	}

	/* it has to read just one integer that is the port name */
	if(sscanf(port_name, DPDKR_FORMAT, &port_no) != 1)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "Port (%s) has bad port name format", port_name);
		return -1;
	}

	/* look for the transmission ring */
	snprintf(ring_name, 20, DPDKR_TX_FORMAT, port_no);
	tx = rte_ring_lookup(ring_name);
	if(tx == NULL)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "port (%s) can not be found", port_name);
		return -1;
	}

	/* look fot the reception ring */
	snprintf(ring_name, 20, DPDKR_RX_FORMAT, port_no);
	rx = rte_ring_lookup(ring_name);
	if(rx == NULL)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "port (%s) can not be found", port_name);
		return -1;
	}

	/* look for the memory pool */
	/*
	* XXX: improve the wasy the memory pool is looked, the name could not
	* always be the same
	*/
	packets_pool = rte_mempool_lookup("ovs_mp_1500_0_262144");
	if(packets_pool == NULL)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "OVS packets mempool can not be found");
		return -1;
	}

	if (rte_ivshmem_metadata_create(port_name) < 0)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "Metadata file can not be created");
		return -1;
	}

	if(rte_ivshmem_metadata_add_ring(tx, port_name) < 0)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "Port (%s) can not be added to metadata_file", port_name);
		return -1;
	}

	if(rte_ivshmem_metadata_add_ring(rx, port_name) < 0)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "Port (%s) can not be added to metadata_file", port_name);
		return -1;
	}

	if(rte_ivshmem_metadata_add_mempool(packets_pool, port_name) < 0)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "OVS packets mmepool can not be added to metadatafile");
		return -1;
	}

	if (rte_ivshmem_metadata_cmdline_generate(cmdline, size, port_name) < 0)
	{
		LOG(ORCH_ERROR, MODULE_NAME, "Command line can not be generated", port_name);
		return -1;
	}

	return 0;
}
