## Native VNF type 5

This file contains the instructions to build a native network function that
realizes a VNF type 5 based on `openvswitch`.

## Requirements

This native network function requires the following package:

  * `openvswitch`
  
It can be installed with the command:

	$ sudo apt-get install openvswitch-switch
	
## Run

Before running this native network function, you must create a tar.gz archive
that contains the files `start`, `stop` and `vswitch.ovsschema` with the
command:

	$ tar -zcf vnft5.tar.gz start stop vswitch.ovsschema

Note that `vnft5` is the name of the network function as specified in the NF-FG
and in the name-resolver configuration file.