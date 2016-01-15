#ifndef DPDK_CONSTANTS_H_
#define DPDK_CONSTANTS_H_ 1

#define DPDK_MODULE_NAME		"DPDK-Process-Manager"

/**
*	@brief: paths of the bash scripts used to manage DPDK
*/
#define PULL_AND_RUN_DPDK_NF	"./compute_controller/plugins/dpdk/scripts/pullAndRunNF.sh"
#define STOP_DPDK_NF			"./compute_controller/plugins/dpdk/scripts/stopNF.sh"

#define NUM_MEMORY_CHANNELS 	2

#endif //DPDK_CONSTANTS_H_
