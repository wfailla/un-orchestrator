# How to launch a virtual network function on the Universal Node

This file describes how to deploy a VNF on the Universal Node, which requires the execution of the following main steps:
*	create the desired virtual network function;
*	register such a network function in the name-resolver database;
*	send a graph description including the VNF to the local orchestrator.

## Create the VNF image
The universal node currently supports three types of virtual network functions: Docker containers, KVM-based virtual machines, and DPDK processes natively run on the universal node itself.  
In order to create your own virtual network function, please check individual README's in each sub-package.

## Register the VNF in the name-resolver
Once the VNF is created, it must be registered in the name-resolver database (to install the name-resolver, please check `../name-resolver/README.md`).  
This operation requires to edit the configuration file of the name-reselver; an example of such a file is available at `../name-resolver/config/example.xml`.

At this point reboot the name-resolver, so that it can read the updated database.

## Provide the graph description to the un-orchestrator
In order to deploy your VNF on the Universal Node, you must provide to the un-orchestration a graph including such a network function.
A description on how to compile the un-orchestator is provided in `../orchestrator/README_COMPILE.md`, while the instructions to execute the un-orchestrator are available at `../orchestrator/README_COMPILE.md`.

The un-orchestrator supports two NF-FG versions:
  * the initial JSON-based format defined in WP5 and used in the initial
    part of the project;
  * the new XML-based format defined in WP3 that includes both top-down
    communication (for the actual forwarding graph) and bottom-up primitives
    (for resources and capabilities).

The former format is supported natively and it is described in `../orchestrator/README_RESTAPI.md`., while the other requires setting up an additional library as described in README_COMPILE.md#nf-fg-library.

### An example

This section shows the steps required to deploy a VNF called `dummy`, which is executed within a Docker container.

To create the VNF image, and store it in the local file system of the universal node, execute the following command:

    sudo docker build --tag="dummy" .
    
Then, register it in the name-resolver by adding the following piece of XML to the configuration file of the name-resolver itself:

	<network-function name="dummy"  num-ports="2" description="dummy VNF used to show how to use the un-orchestrator">
		<implementation type="docker" uri="dummy"/>
	</network-function>

At this point, to deploy the following graph on the universal node

![service-graph](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/master/images/service-graph.png)

send the following JSON to the un-orchestator:

  
    {
        "flow-graph": {  
            "VNFs": [  
            {  
                "id": "dummy"  
            }  
            ],  
            "flow-rules": [  
            {  
                "id": "00000001",  
                "match": 
                {  
                    "port": "eth1"  
                },  
                "action": 
                {  
                    "VNF_id": "dummy:1"  
                }  
            },  
            {  
                "id": "00000002",  
                "match": 
                {  
                    "VNF_id": "dummy:2"  
                },  
                "action": 
                {  
                    "port": "eth2"  
                }  
            },  
            {  
                "id": "00000003",  
                "match": 
                {  
                    "port": "eth2"  
                },  
                "action": 
                {  
                    "VNF_id": "dummy:2"  
                }  
            },  
            {  
                "id": "00000004",  
                "match": 
                {  
                    "VNF_id": "dummy:1"  
                },  
                "action": 
                {      
                    "port": "eth1"  
                }  
            } 
            ]  
        }  
    }  
