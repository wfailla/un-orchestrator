# DPDK-based VNF examples

This folder contains examples of network functions implemented as DPDK secondary
processes.

## How to install DPDK

DPDK can be installed along with the virtual switch. It is in fact mandatory in case of
xDPd, while it is optional in case of OvS.

#### xDPd

In order to install xDPd and the DPDK library, you have to follow the steps below.

	$ git clone https://github.com/bisdn/xdpd  
	$ cd xdpd/  

	;Install all the libraries required by the README provided in this folder  
	$ bash autogen  
	$ cd build  
	$ ../configure --with-hw-support=gnu-linux-dpdk --with-plugins="node_orchestrator rest"   
	$ make
	$sudo make install

Now the DPDK library, which is being used by xDPd, must be properly
configured, which can be done by launching a script that allows you to:

  * build the environment x86_64-native-linuxapp-gcc
  * Insert IGB UIO module
  * Insert KNI module
  * Setup hugepage mappings for non-NUMA systems (1000 should be a
    reasonable number)
  * Bind Ethernet device to IGB UIO module (bind all the ethernet
    interfaces that you want to use)

Let's now launch the DPDK setup script:

	$ cd ../libs/dpdk/tools  
	$ sudo ./setup.sh  

**WARNING: Currently, xDPd is not compiling on Linux kernels newer than 3.16.0-30.**

#### Open vSwitch (OVSDB)

At first, downaload the Open vSwitch source code from:

    http://openvswitch.org/releases/openvswitch-2.4.0.tar.gz

Then execute the following commands to extract the archive

    $ tar -xf openvswitch-2.4.0.tar.gz
    
Now, compile OvS as described in the file README.DPDK provided in the downloaded archive. Note that
OvS **must** be compiled with the IVSHMEM support.
        
## How to create your VNFs

Please check individual README's in each sub-package.
Those files will give you the instruction to create the binary for the selected VNF.
Once you have that binary, you can pass the link to the UN (by writing the appropriate entry in the name resolver configuration file) in order to instantiate it in your running environment.
