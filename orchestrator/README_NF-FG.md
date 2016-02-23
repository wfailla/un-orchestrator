# NF-FG examples
This section provides some examples of NF-FGs that can be deployed on the Universal
Node through the un-orchestrator.

Further examples are available in the [./config](./config) folder and at [https://github.com/netgroup-polito/nffg-library/blob/master/README_NFFG](https://github.com/netgroup-polito/nffg-library/blob/master/README_NFFG).

## Example 1

This example is very simple: configures a graph that receives all the traffic
from interface `eth1` and sends it to interface `eth2`, without traversing any
VNF.

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
		      		"interface": "eth1"
		    	}
		  	},
		  	{
		    	"id": "00000002",
		    	"name": "egress",
		    	"type": "interface",
		    	"interface": {
		      		"interface": "eth2"
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
		    	}
		  		]
			}
	  	}
	}
	
## Example 2

This example is more complex, and it includes a network function called `firewall`.
Packets coming from the interface `eth1` are sent to the first port of the network
function (`firewall:1`), while packets coming from the second port of the network
function (`firewall:2`) are sent on the network interface `eth2`.

	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Forwarding graph",
			"VNFs": [
		  	{
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
		    	"id": "00000001",
		    	"name": "ingress",
		    	"type": "interface",
		    	"interface": {
		      		"interface": "eth1"
		    	}
		  	},
		  	{
		    	"id": "00000002",
		    	"name": "egress",
		    	"type": "interface",
		    	"interface": {
		      		"interface": "eth2"
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
		          		"output_to_port": "endpoint:00000002"
		        	}
		      		]
		    	}
		  		]
			}
	  	}
	}
	
## Example 3

In this example, traffic coming from `eth1` is forwarded to the `firewall` through the port
`firewall:1`. Then, traffic coming from the firewall (`firewall:2`) is split based on the destination
TCP port. Packets directed to the TCP port 80 is provided to the web cache then to the NAT,
while all the other traffic is directly provided to the NAT. Finally, packets from `NAT:2` leaves the
graph through the port `eth2`.

This graph can be graphically represented as follows:

    eth1 -> firewall -> if (tcp_dst == 80) -> web cache  -> nat  -> eth2
                        else \--------------------------/

Json description of the graph:

	{
		"forwarding-graph": 
		{
			"id": "00000001",
			"name": "Forwarding graph",
			"VNFs": [
		  	{
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
		      		"interface": "eth1"
		    	}
		  	},
		  	{
		    	"id": "00000005",
		    	"name": "egress",
		    	"type": "interface",
		    	"interface": {
		      		"interface": "eth2"
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

It is possible to send/receive traffic through GRE tunnels, by using the "gre-tunnel" endpoint. 
For instance, this feature can be used to connect together pieces of the same service deployed 
on different Universal Nodes.

This example sends on a GRE tunnel all the traffic received through the physical port `eth1`, and 
vice versa. This graph can be graphically represented as follows:

    eth1 <-> gre-tunnel
   
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
		      		"interface": "eth1"
		    	}
		  	},
		  	{
		    	"id": "00000002",
		    	"name": "egress",
		    	"type": "gre-tunnel",
		    	"gre": {
		      		"local-ip": "10.0.0.1",
		      		"remote-ip": "10.0.0.2",
		      		"interface" : "eth1",
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

	"port_in"
	"ether_type"
	"source_mac"
	"eth_src_mask"
	"dest_mac"
	"eth_dst_mask"
	"source_ip"
	"dest_ip"
	"source_port"
	"dest_port"
	"protocol"
	"vlan_id"
	"vlan_pcp"
	"ip_dscp"
	"ip_ecn"
	"ipv4_src"
	"ipv4_src_mask"
	"ipv4_dst"
	"ipv4_dst_mask"
	"sctp_src"
	"sctp_dst"
	"icmpv4_type"
	"icmpv4_code"
	"arp_opcode"
	"arp_spa"
	"arp_spa_mask"
	"arp_tpa"
	"arp_tpa_mask"
	"arp_sha"
	"arp_tha"
	"ipv6_src"
	"ipv6_src_mask"
	"ipv6_dst"
	"ipv6_dst_mask"
	"ipv6_flabel"
	"ipv6_nd_target"
	"ipv6_nd_sll"
	"ipv6_nd_tll"
	"icmpv6_type"
	"icmpv6_code"
	"mpls_label"
	"mpls_tc"
        
## Supported actions

Within the `action` element of the NF-FG description, one and only one of the
following fields **MUST** be specified:

	"output_to_port"
	"push_vlan"
	"pop_vlan"
  
As an example, the following NF-FG tags all the packets coming from interface `eth1` and forwards them on interface `eth2`.

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
		      		"interface": "eth1"
		    	}
		  	},
		  	{
		    	"id": "00000002",
		    	"name": "egress",
		    	"type": "vlan",
		    	"vlan": {
		      		"vlan-id": "25",
		      		"interface": "eth2"
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
 
