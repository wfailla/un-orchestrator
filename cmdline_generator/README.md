# cmdline-generator

The cmdline-generator is a module that provides the necessary command line to connect a dpdkr port type to QEMU.

## Building

1. Install the required libraries

	`$ sudo apt-get install build-essential cmake libboost-all-dev libmicrohttpd-dev `
	
2. Install json-spirit

    1. `$ git clone https://github.com/sirikata/json-spirit`
	2. `$ cd json-spirit/`
	3. `$ cmake .`
	4. `$ make`
	5. `$ ./json_test [optional step] `
	6. `$ sudo make install`

3. Build DPDK (If you have already built it for compiling ovs with dpdk support then you can jump this step)
	1. Set `$DPDK_DIR` to the folder where you have dpdk
	2. `$ cd $DPDK_DIR`
	3. Edit config/common_linuxapp to generate a single lib file.
	
		`CONFIG_RTE_BUILD_COMBINE_LIBS=y`
		1. `$make install`
		2. `$make install T=x86_64-ivshmem-linuxapp-gcc`
	
4. Build cmdline-generator
	1. Ensure that the `$DPDK_DIR` environmental path is still valid
	2. `$ make`
     
# How to run the cmdline-generator module

The cmdline-generator module does not receives any command line option, it just requires to be run as sudo.

	$sudo ./cmdline_generator
	
Once it is running, it will answer http requests on the port 6666, the request must use the GET method and indicate the port to which generate the command line in the URL.
Please note that OVS must be running.

	$ curl http://127.0.0.1:6666/dpdkr2
	
The answer will be formatted using JSON as 
`{
    "Command": "command to the introduce in QEMU"
}`, If and error occurs the module will close the connection without responding.
