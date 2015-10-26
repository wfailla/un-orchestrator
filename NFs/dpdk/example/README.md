## DPDK-based example VNF

This folder contains the instructions to build a simple network function that exchanges packets with the virtual switch
through the DPDK rte_rings.

When the VNF receives a packet on the interface 'i', it destination MAC address is modified and the packet is sent sent
on the interface 'i+1'.

### Creating the binary

In addition to the DPDK library, the DPI requires the pcre library to implements the packet inspections. 
The pcre library can be installed with the command:

	$ sudo apt-get install libpcre3 libpcre3-dev

To compile the DPI, run the following commands:

	$ export RTE_SDK=absolute_path_dpdk  
	; e.g., export RTE_SDK=~/Desktop/dpdk-dev
	$ make
