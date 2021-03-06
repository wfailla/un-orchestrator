# REST API

The un-orchestrator can either accept a NF-FG from a file, or from its REST interface,
that thanks to a small HTTP server embedded in the UN. The main REST commands to be used
to interact with the un-orchestrator (e.g., deploy a new graph, update an existing graph,
etc.) are detailed in this document.

## Main REST commands accepted by the un-orchestrator

Deploy an NF-FG called ``myGraph'' (the NF-FG description must be based on the
formalism defined in WP5 [README_NF-FG.md](README_NF-FG.md))

    PUT /NF-FG/myGraph HTTP/1.1
    Content-Type : application/json

    {
		"forwarding-graph": {
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
				}
			  ]
			}
		}
	}

The same message used to create a new graph can be used to add "parts" (i.e.,
network functions and flows) to an existing graph. For instance, it is possible
to add a new flow to the NF-FG called ``myGraph'' as follows

    PUT /NF-FG/myGraph HTTP/1.1
    Content-Type : application/json

    {
		"forwarding-graph": {
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
				"id": "00000002",
				"name": "ingress",
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

Retrieve the description of the graph with name "myGraph":

	GET /NF-FG/myGraph HTTP/1.1

Delete the graph with name "myGraph"

	DELETE /NF-FG/myGraph HTTP/1.1

Delete the flow with ID "flow_id" from the graph with name "myGraph":

	DELETE /NF-FG/myGraph/flow_id HTTP/1.1

Retrieve information on the available physical interfaces:

	GET /interfaces HTTP/1.1

## User authentication

The un-orchestrator supports user authentication, which has to be enabled through the configuration file of the module.
In case this feature is turned on, all the interactions with the UN must start with an authentication message, which looks like the following:

    POST /login	HTTP/1.1
    Content-Type : application/json

    {
        "username":"admin", 
        "password":"admin"
    }
    
If the authentication is successful, this method will return a token in the response.

The returned token has to be used by creating an additional `X-Auth-Token` header in all the requests you send to the UN.
In this way the UN will know the identity of the user and it will be able to check whether the user has the right to perform the requested operation or not.

Users and permissions are stored in a local SQLite database.

## Send commands to the un-orchestrator

In order to interact with the un-orchestrator through its REST API, you can use
your favorite REST tool (e.g., some nice plugins for Mozilla Firefox). Just in
also use the CURL command line tool, such as in the following example (where the
NF-FG to be instantiated is stored in the file 'myGraph.json'):

	$ curl -i -H "Content-Type: application/json" -d "@myGraph.json" \
		-X PUT  http://un-orchestrator-address:port/NF-FG/myGraph
		

