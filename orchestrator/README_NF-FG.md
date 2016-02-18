# NF-FG examples
This section provides some examples of NF-FGs that can be deployed on the Universal
Node through the un-orchestrator.

Further examples are available in the `config` folder and at [https://github.com/netgroup-polito/nffg-library/blob/master/README_NFFG](https://github.com/netgroup-polito/nffg-library/blob/master/README_NFFG).

## Example 1

This example is very simple: configures a graph that receives all the traffic
from interface `eth0` and sends it to interface `eth1`, without traversing any
VNF.

	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Forwarding graph",
			"end-points": [
		  	{
		    	"id": "00000003",
		    	"name": "ingress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.1",
		      		"interface": "eth0"
		    	}
		  	},
		  	{
		    	"id": "00000004",
		    	"name": "egress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.2",
		      		"interface": "eth1"
		    	}
		  	}
			],
			"big-switch": {
		  		"flow-rules": [
		    	{
		      	    "id": "000000001",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000003"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000004"
		        	}
		      		]
		    	}
		  		]
			}
	  	}
	}
	
## Example 2

This example is more complex, and it includes a network function called "firewall".
Packets coming from the interface `eth0` are sent to the first port of the network
function (`firewall:1`), while packets coming from the second port of the network
function (`firewall:2`) are sent on the network interface `eth1`.

	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Forwarding graph",
			"VNFs": [
		  	{
		    	"vnf_template": "firewall.json",
		    	"id": "00000001",
		    	"name": "firewall",
        		"ports": [
          		{
            		"id": "inout:0",
            		"name": "data-port"
          		},
          		{
            		"id": "inout:1",
            		"name": "data-port"
          		}
        		]
		  	}
			],
			"end-points": [
		  	{
		    	"id": "00000002",
		    	"name": "ingress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.1",
		      		"interface": "eth0"
		    	}
		  	},
		  	{
		    	"id": "00000003",
		    	"name": "egress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.2",
		      		"interface": "eth1"
		    	}
		  	}
			],
			"big-switch": {
		  		"flow-rules": [
		    	{
		      		"id": "000000001",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000002"
		      		},
		      		"actions": [
		        	{
		        		"output_to_port": "vnf:00000001:inout:0"
		        	}
		      		]
		    	},
		    	{
		      		"id": "000000002",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "vnf:00000001:inout:1"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000003"
		        	}
		      		]
		    	}
		  		]
			}
	  	}
	}
	
## Example 3

In this example, traffic coming from `eth0` is forwarded to the firewall through the port
`firewall:1`. Then, traffic coming from the firewall (`firewall:2`) is split based on the destination
TCP port. Packets directed to the TCP port 80 is provided to the web cache then to the NAT,
while all the other traffic is directly provided to the NAT. Finally, packets from `NAT:2` leaves the
graph through the port `eth2`.

This graph can be graphically represented as follows:

    eth0 -> firewall -> if (tcp_dst == 80) -> web cache  -> nat  -> eth1
                        else \--------------------------/

Json description of the graph:

	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Forwarding graph",
			"VNFs": [
		  	{
		    	"vnf_template": "firewall.json",
		    	"id": "00000001",
		    	"name": "firewall",
        		"ports": [
          		{
            		"id": "inout:0",
            		"name": "data-port"
          		},
          		{
            		"id": "inout:1",
            		"name": "data-port"
          		}
        		]
		  	},
		  	{
		    	"vnf_template": "nat.json",
		    	"id": "00000002",
		    	"name": "nat",
        		"ports": [
          		{
            		"id": "inout:0",
            		"name": "data-port"
          		},
          		{
            		"id": "inout:1",
            		"name": "data-port"
          		}
        		]
		  	},
		  	{
		    	"vnf_template": "web-cache.json",
		    	"id": "00000003",
		    	"name": "web-cache",
        		"ports": [
          		{
            		"id": "inout:0",
            		"name": "data-port"
          		},
          		{
            		"id": "inout:1",
            		"name": "data-port"
          		}
        		]
		  	}
			],
			"end-points": [
		  	{
		    	"id": "00000004",
		    	"name": "ingress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.1",
		      		"interface": "eth0"
		    	}
		  	},
		  	{
		    	"id": "00000005",
		    	"name": "egress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.2",
		      		"interface": "eth1"
		    	}
		  	}
			],
			"big-switch": {
		  		"flow-rules": [
		    	{
		      		"id": "000000001",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000004"
		      		},
		      		"actions": [
		        	{
		        		"output_to_port": "vnf:00000001:inout:0"
		        	}
		      		]
		    	},
		    	{
		      		"id": "000000002",
		      		"priority": 10,
		      		"match": {
		        		"port_in": "vnf:00000001:inout:1",
            			"protocol": "tcp",
            			"dest_port": "80"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "vnf:00000003:inout:0"
		        	}
		      		]
		    	},
		    	{
		      		"id": "000000003",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "vnf:00000001:inout:1"
		      		},
		      		"actions": [
		        	{
		        		"output_to_port": "vnf:00000002:inout:0"
		        	}
		      		]
		    	},
		    	{
		      		"id": "000000004",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "vnf:00000003:inout:1"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "vnf:00000002:inout:0"
		        	}
		      		]
		    	},
		    	{
		      		"id": "000000005",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "vnf:00000002:inout:1"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000005"
		        	}
		      		]
		    	}
		  		]
			}
	  	}
	}
	
## Example 4

It is possible to connect multiple universal nodes together by using the "gre-tunnel" endpoint.

This graph can be graphically represented as follows:

    eth0 <-> gre-tunnel
   
Json description of the graph:
 
	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Forwarding graph",
			"end-points": [
			{
		    	"id": "00000001",
		    	"name": "ingress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.1",
		      		"interface": "eth0"
		    	}
		  	},
		  	{
		    	"id": "00000002",
		    	"name": "egress",
		    	"type": "gre-tunnel",
		    	"gre": {
		      		"local-ip": "10.0.0.1",
		      		"remote-ip": "10.0.0.2",
		      		"interface" : "eth0",
		      		"gre-key" : "1"
		    	}
		  	}
			],
			"big-switch": {
		  		"flow-rules": [
		    	{
		      		"id": "000000001",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000001"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000002"
		        	}	
		      	]
		    	},
		    	{
		     		"id": "000000002",
		     		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000002"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000001"
		        	}
		      		]
		    	}
		  		]
			}
	  	}
	}
	
## Supported matches

Within the `match` element of the NF-FG description, the following fields are allowed
(all the values must be specified as strings):

    "ether_type"
    "vlan_id"
    "source_mac"
    "dest_mac"
    "source_ip"
    "dest_ip"
    "source_port"
    "dest_port"
    "protocol"
    "port_in"
        
## Supported actions

Within the `action` element of the NF-FG description, one and only one of the
following fields **MUST** be specified:

	"output_to_port"
    "push_vlan"
    "pop_vlan"
  
As an example, the following NF-FG tags all the packets coming from interface `eth0` and forwards them on interface `eth1`.

	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Only network graph",
			"domain" : "domain1",
			"end-points": [
		  	{
		    	"id": "00000001",
		    	"name": "ingress",
		    	"type": "interface",
		    	"interface": {
		      		"node-id": "10.0.0.1",
		      		"interface": "eth0"
		    	}
		  	},
		  	{
		    	"id": "00000002",
		    	"name": "egress",
		    	"type": "vlan",
		    	"vlan": {
		      		"vlan-id": "25",
		      		"node-id": "10.0.0.1",
		      		"interface": "eth1"
		    	}
		  	}
			],
			"big-switch": {
		 		"flow-rules": [
		   	 	{
		      		"id": "000000001",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000001"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000002"
		        	}
		      		]
		    	},
		    	{
		      		"id": "000000002",
		      		"priority": 1,
		      		"match": {
		        		"port_in": "endpoint:00000002"
		      		},
		      		"actions": [
		        	{
		          		"output_to_port": "endpoint:00000001"
		        	}
		      		]
		    	}
		  	]
			}
	  	}
	}
 
