#Configuration file of the virtualizer
CONFIGURATION_FILE = 'config/configuration.ini'

'''
	File used by the virtualizer to maintain the state (i.e., rules deployed,
	VNF instantiated
'''

#TMP file use by the virtualizer to maintain the current configuration of the node
GRAPH_XML_FILE = '.universalnode.xml'

#TMP file used by the virtualizer and representing the deployed graph,
#in the JSON syntax internally used by the virtualizer itself
GRAPH_FILE = '.graph.json'

'''
	Information to be exported
'''
INFRASTRUCTURE_NAME = 'Single node'
INFRASTRUCTURE_ID = 'UUID001'
NODE_NAME = 'Universal Node'
NODE_ID = 'UUID11'
NODE_TYPE = 'BisBis'

'''
	Supported matches.
	For each key, it indicates the equivalent match in the NF-FG supported natively
	by the node virtualizer.
'''
supported_matches = {
	"dl_vlan" : "vlan_id"
}

'''
	Supported actions.
	For each key, it indicates the number of parameters required (0 or 1).
'''
#XXX: for each key, there must be an handler for that key in actionsParser.py
supported_actions = {
	"strip_vlan" : 0,
	"push_vlan" : 1
}

'''
	Equivalent actions.
	For each key, it indicates the equivalent action in the NF-FG supported natively
	by the node virtualizer.
'''
#XXX: for each key, there must be an handler for that key in actionsParser.py
equivalent_actions = {
	"strip_vlan" : "vlan",
	"push_vlan" : "vlan"
}

