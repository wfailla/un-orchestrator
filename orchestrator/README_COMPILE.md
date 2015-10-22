# How to compile the un-orchestrator

In order to execute the un-orchestrator, we need to setup different components, namely:

  * a set of libraries needed to compile the un-orchestrator code
  * a virtual switch (either xDPd or OpenvSwitch) as a base switch for
    our platform
  * an execution environment for virtual network functions, e.g., KVM for
    executing VM, Docker, or other.

### Required libraries

Several libraries are required to compile the un-orchestrator.
In the following we list the steps required on an **Ubuntu 14.04**.

	; Install required libraries
	; - build-essential: it includes GCC, basic libraries, etc
	; - cmake: to create cross-platform makefiles
	; - cmake-curses-gui: nice 'gui' to edit cmake files
	; - libboost-all-dev: nice c++ library with tons of useful functions
	; -libmicrohttpd-dev: embedded micro http server
	$ sudo apt-get install build-essential cmake cmake-curses-gui libboost-all-dev libmicrohttpd-dev

	; Install JSON Spirit (nice library to parse JSON files)
	$ git clone https://github.com/sirikata/json-spirit
	$ cd json-spirit/

	; Now install the above library according to the description provided
	; in the cloned folder

	; Install ROFL-common  (library to parse OpenFlow messages)
	$ git clone https://github.com/bisdn/rofl-common  
	$ cd rofl-core/

	; Now install the above library according to the description provided
	; in the cloned folder

### Install the proper virtual switch

The current un-orchestrator supports different types of virtual switches.
You have to install the one that you want to use, choosing from the
possibilities listed in this section.


#### xDPd

In order to install xDPd, you have to follow the steps below.

	git clone https://github.com/bisdn/xdpd  
	cd xdpd/  

	;Install all the libraries required by the README provided in this folder  
	bash autogen  
	cd build  
	../configure --with-hw-support=gnu-linux-dpdk --with-plugins="node_orchestrator rest"   
	make

Now the DPDK library, which is being used by xDPd, must be properly
configured, which can be done by launching a script that allows you to:

  * build the environment x86_64-native-linuxapp-gcc
  * Insert IGB UIO module
  * Insert KNI module
  * Setup hugepage mappings for non-NUMA systems (1000 could be a
    reasonable number)
  * Bind Ethernet device to IGB UIO module (bind all the ethernet
    interfaces that you want to use)

Let's now launch the DPDK setup script (note that the library has been downloaded
togher with xDPd, and it is located at xdpd/libs/dpdk):

	$ cd ../libs/dpdk/tools  
	$ sudo ./setup.sh  


#### Open vSwitch (of-config) [DEPRECATED]

OpenvSwitch can be installed with either the OVSDB or OF-CONFIG plugins.
Although both protocols allow to control the switch (e.g., create/delete
new bridging instances, create/delete ports, etc), we found out
that OF-CONFIG is rather limited in terms of capabilities. For instance,
it cannot set the type of port configured on the switch (e.g., virtio
or IVSHMEM), requiring the orchestrator to rely on a combination of
OF-CONFIG commands and bash scripts to perform its job.

For this reason we suggest to install OpenvSwitch with its native OSVDB 
support (next section); although OVSDB is not standard, it seems that it
does its job better than OF-CONFIG.

In any case, the compilation instruction for setting up OpenvSwitch with 
OF-CONFIG are the following (not guaranteed that those are 100% accurate,
as the OF-CONFIG support in OpenvSwitch is rather primitive).

The list of OF-CONFIG dependencies:

- libnetconf 0.9.x, not higher
- compiler (gcc, clang,...) and standard headers
- pkg-config
- libpthreads
- libxml2 (including headers from the devel package)
- libssh >= 0.6.4 (including headers)
	- download it from https://red.libssh.org/projects/libssh/files and for install it 
	  following the INSTALL file present in the root directory
	- this step can be skipped if using --disable-ssh
- openvswitch 2.4.0
- pyang >= 1.5.0
- python 2.6 or higher with the following modules:
 - os, copy, string, re, argparse, subprocess, inspect, curses, xml, libxml2
 - only with TLS enabled: M2Crypto
- only with TLS enabled by using the --enable-tls option
 - OpenSSL (libssl, libcrypto, including headers from the devel package)
- roff2html
 - optional, used for building HTML version of man pages (make doc)
- rpmbuild
 - optional, used for building RPM package (make rpm).

Compile and install libnetconf as described here, including headers from the devel package:

	; Clone the libnetconf repository
	$ git clone https://github.com/cesnet/libnetconf
    $ cd libnetconf/
    $ git checkout -b 0.9.x origin/0.9.x

Install the libnetconf library by following the instructions in the
INSTALL file contained in the root folder of this library.

You can now install of-config:

	; Clone the openvswitch repository
	$ git clone https://github.com/openvswitch/of-config    

Follow the instructions as described in the file INSTALL.md provided
in the root folder of that repository.


#### Open vSwitch (OVSDB)

######Open vSwitch Installation

At first, downaload the Open vSwitch source code from:

    http://openvswitch.org/releases/openvswitch-2.4.0.tar.gz

Then execute the following commands:

    $ tar -xf openvswitch-2.4.0.tar.gz
    $ cd openvswitch-2.4.0
    $ ./configure --prefix=/ --datarootdir=/usr/share --with-linux=/lib/modules/$(uname -r)/build
    $ make
    $ sudo make install

To start Open vSwitch at the boot of the machine (optional):

    $ sed 's,/usr/share/,/usr/local/share/,' rhel/etc_init.d_openvswitch > /etc/init.d/openvswitch
    $ chkconfig --add openvswitch
    $ chkconfig openvswitch on

Note: sed(1) is used to rewrite path to Open vSwitch scripts that is statically defined
in openvswitch script.


### Virtual Execution Environment for network functions

The current un-orchestrator supports different types of execution environments.
You have to install the ones that you want to use, selecting one or more
possibilities from the ones listed in this section.

#### Docker

In order to suppor the Docker execution environment, first follow the instructions
provided here:

	http://docs.docker.com/installation/  

Then executes the following commands to properly configure the Docker environment:

	$ sudo apt-get install lxc -y  
	$ echo 'DOCKER_OPTS="-e lxc"' >> /etc/default/docker  
	$ service docker restart
	
**WARNING: the Docker execution environment is only supported when using xDPd 
as virtual switch.**

#### Libvirt (and KVM)

This is needed in order to run network functions in KVM-based virtual machines.
To compile and install libvirt, execute the following command:

	$ sudo apt-get install libvirt-dev qemu-kvm libvirt-bin bridge-utils  

If you run Libvirt for OVS or OVSDB, please put your template in the folder 
"compute_controller/plugins/kvm-libvirt/nf_repository".

**WARNING: the KVM execution environment is only supported when using OvS 
as virtual switch.**

### NF-FG library

These steps are mandatory only if you plan to use the Network Functions - 
Forwarding Graph (NF-FG) defined in WP3.

	; Retrieve the NF-FG library.
	
	; Copy the library in the un-orchestrator folder
	$ cp [nffg]/virtualizer3.pyc [un-orchestrator]/orchestrator/node_resource_manager/virtualizer      

### Compile the un-orchestrator

We are now ready to compile the un-orchestrator.

	$ cd orchestrator

	; Choose among possible compilation options
	$ ccmake .  

The previous command allows you to select some configuration parameters for the
un-orchestrator, such as the virtual switch used, which kind of execution environment
you want to enable, the NF-FG description, etc. When you're finished, exit from
the 'ccmake' interface by *generating the configuration files* (press 'c' and 'g')
and type the following commands:

	; Create makefile scripts based on the previously selected options
	$ cmake .

	; Compile and create the executables
	$ make
