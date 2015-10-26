## DPDK-based DPI VNF

This folder contains the instructions to build a DPI that exchanges packets with the virtual switch
through the DPDK rte_rings.

The virtual network function analyzes packets coming from the first interfaces and, if a packet is
allowed, forwards it on the second interface. Packets flowing in the opposite direction are instead
forwarded without any analysis.

By default, the DPI drops the HTTP containing the words "porn" and "sex".

### Configuring the DPI

The configuration of the DPI has to be embedded in the source code; right now, we cannot change dynamically (at run-time) the configuration.

To change the configuration of the DPI (e.g., changing the network interfaces it binds to, the rules used to allow/block the traffic, etc.), edit the file `runtime.c`.

### Creating the binary

In addition to the DPDK library, the DPI requires the pcre library to implements the packet inspections. 
The pcre library can be installed with the command:

	$ sudo apt-get install libpcre3 libpcre3-dev

To compile the DPI, run the following commands:

	$ export RTE_SDK=absolute_path_dpdk  
	; e.g., export RTE_SDK=~/Desktop/dpdk-dev
	$ make
