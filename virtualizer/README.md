# Virtualizer

The virtualizer is an intermediate module sit between the un-orchestrator and 
the upper layer orchestrator. It operates as follows:
  * receives commands from the upper layer orchestrator based on the virtualizer 
    library defined by WP3;
  * converts those command in the formalism natively supported by the un-orchestrator
    (described in [../orchestrator/README_NF-FG.md](../orchestrator/README_NF-FG.md));
  * sends the command to the un-orchestrator through the API described in 

The virtualizer module is only required if you plan to use the Network Functions - 
Forwarding Graph (NF-FG) defined in WP3, which is based on the concept of *virtualizer*.


## Required libraries

In addition to the libraries already listed in the main [../README_COMPILE.md](../README_COMPILE.md),
some more components are required to compile the un-orchestrator.

	; Retrieve the virtualizer library and the library used to handle the NF-FG in the format natively supported by the un-orchestrator.
	$ cd [un-orchestrator]
	$ git submodule update --init --recursive

	; Install other required libraries 
	$ sudo apt-get install python-pip
	$ sudo pip install gunicorn falcon cython enum

## How to configure the virtualizer

The virtualizer reads its configuration from the file [./config/configuration.ini](config/configuration.ini), 
which must be properly edited before starting the virtualizer itself.

## How to run the virtualizer

	$ gunicorn -b ip:port example:api

where 'ip' and 'port' must be set to the desired values.

Please, note that the virtualizer requires the un-orchestrator and the 
name-resolver running in the server.

## Rest API

The virtualizer accept commands through its REST interface. The main REST commands 
to be used to interact with it are detailed in the following.

Get information about the virtualizer

    GET / HTTP/1.1
    
Test the virtualizer aliveness

    GET /ping HTTP/1.1

Retrieve the current configuration of the universal node

    POST /get-config HTTP/1.1

This command returns the current configuration in the format defined by the virtualzier library

Deploy a new configuration 

    POST /edit-config HTTP/1.1

The body of the message must contain the new configuration for the universal node 
espressed in the format defined by the virtualizer library.

Examples of configurations that can be sent to the virtualizer are available in [./config/nffg_examples](nffg_examples).
In particular:
  * [./config/nffg_examples/simple_passthrough_nffg.xml](./config/nffg_examples/simple_passthrough_nffg.xml): 
    simple configuration that implements a simple passthrough function, i.e., traffic is 
    received from a first physical port and sent out from a second physical port, 
    after having been handled to the vswitch;
  * [./config/nffg_examples/passthrough_with_vnf_nffg.xml](./config/nffg_examples/passthrough_with_vnf_nffg.xml): 
    configuration that includes a VNF. Traffic is received from a first physical 
    port, provided to a network function, and then sent out from a second physical 
    port;
  * [./config/nffg_examples/passthrough_with_vnf_nffg_and_match_and_action.xml](./config/passthrough_with_vnf_nffg_and_match_and_action.xml): 
    this configuration includes flows matching some protocol fields, and having 
    actions that manipulate protocol fields;
  * [./config/nffg_examples/nffg_delete_flow_vnf.xml](./config/nffg_examples/nffg_delete_flow_vnf.xml): 
    configuration that deletes some flows and a VNF instantiated on the universal 
    node.
    
