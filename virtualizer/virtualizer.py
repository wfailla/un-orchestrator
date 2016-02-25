#!/usr/bin/env python
__author__ = 'Ivano Cerrato, Stefano Petrangeli'

import falcon
import json
import logging
import copy
import requests
from enum import Enum
import ConfigParser

import constants
from virtualizer_library.virtualizer3 import ET, Virtualizer,  Software_resource, Infra_node, Port as Virt_Port
from un_native_nffg_library.nffg import NF_FG, VNF, Match, Action, EndPoint, FlowRule, Port

class Error(Enum):
	noerror = 0
	client = 4
	server = 5

class DoPing:
	'''
	This class manages the ping
	'''

	def on_get(self,req,resp):
		resp.body = "OK"
		resp.status = falcon.HTTP_200
		
	def on_post(self,req,resp):
		resp.body = "OK"
		resp.status = falcon.HTTP_200

class DoUsage:
	'''
	This class shows how to interact with the virtualizer
	'''
	
	def __init__(self):
		a = 'usage:\n'
		b = '\tget http://hostip:tcpport - this help message\n'
		c = '\tget http://hostip:tcpport/ping - test webserver aliveness\n'
		d = '\tpost http://hostip:tcpport/get-config - query NF-FG\n'
		e = '\tpost http://hostip:tcpport/edit-config - send NF-FG request in the post body\n'
		f = '\n'
		g = 'limitations (of the universal node orchestrator):\n'
		h = '\tthe flowrule ID must be unique on the node.\n'
		i = '\ttype cannot be repeated in multiple NF instances.\n'
		j = '\tcapabilities are not supported.\n'
		k = '\tit is not possible to deploy a VNF that does not have any flow referring its ports.\n'	
		l = '\ta VNF is removed (undeployed) only when no flows still remain that refer to its ports.\n'
		m = '\tthe number of ports actually attached to a VNF depends on the number of ports used in the flows'
		N = '\n\n'
		self._answer = a + b + c + d + e + f + g + h + i + j + k + l + m + N

	def on_get(self,req,resp):
		resp.body = self._answer
		resp.status = falcon.HTTP_200
		
	def on_post(self,req,resp):
		resp.body = self._answer
		resp.status = falcon.HTTP_200

class DoGetConfig:

	def on_post(self,req,resp):
		'''
		Return the current configuration of the node.
		'''
		LOG.info("Executing the 'get-config' command")
		LOG.debug("Reading file: %s",constants.GRAPH_XML_FILE)

		try:
			tree = ET.parse(constants.GRAPH_XML_FILE)
		except ET.ParseError as e:
			print('ParseError: %s' % e.message)
			resp.status = falcon.HTTP_500
			return
			
		LOG.debug("File correctly read")
	
		infrastructure = Virtualizer.parse(root=tree.getroot())
	
		LOG.debug("%s",infrastructure.xml())

		resp.body = infrastructure.xml()
		resp.status = falcon.HTTP_200

		LOG.info("'get-config' command properly handled")

class DoEditConfig:
	
	def on_post(self,req,resp):
		'''
		Edit the configuration of the node
		'''
		LOG.info("Executing the 'edit-config' command")
		content = req.stream.read()
		##content = req
		
		LOG.debug("Body of the request:")
		LOG.debug("%s",content)
		
		#TODO: check that flows refer to existing (i.e., deployed) network function.
		#TODO: check that flows refer to existing ports 
		
		#FIXME: not clear what this function actually does
		if not isCorrect(content):
			resp.status = falcon.HTTP_400
			return	
		
		error = Error.noerror
		
		#
		#	Extract the needed information from the message received from the network
		#
		
		vnfsToBeAdded = extractVNFsInstantiated(content)	#VNF deployed/to be deployed on the universal node
		if error == Error.client:
			resp.status = falcon.HTTP_400
			return
		if error == Error.server:
			resp.status = falcon.HTTP_500
			return
			
		rules, endpoints = extractRules(content)			#Flowrules and endpoints installed/to be installed on the universal node
		if error == Error.client:
			resp.status = falcon.HTTP_400
			return
		if error == Error.server:
			resp.status = falcon.HTTP_500
			return
			
		vnfsToBeRemoved = extractToBeRemovedVNFs(content)	#VNFs to be removed from the universal node
		if error == Error.client:
			resp.status = falcon.HTTP_400
			return
		if error == Error.server:
			resp.status = falcon.HTTP_500
			return
			
		rulesToBeRemoved = extractToBeRemovedRules(content) #Rules to be removed from the universal node
		if error == Error.client:
			resp.status = falcon.HTTP_400
			return
		if error == Error.server:
			resp.status = falcon.HTTP_500
			return
			
		#Selects, among the rules listed in the received configuration, those that are not 
		#installed yet in the universal node
		rulesToBeAdded = diffRulesToBeAdded(rules)
		if error == Error.client:
			resp.status = falcon.HTTP_400
			return
		if error == Error.server:
			resp.status = falcon.HTTP_500
			return		
		#XXX The previous operation is not done for VNFs, since the universal node orchestrator supports such a case	
	
		#
		# Interact with the universal node orchestrator in order to implement the required commands
		#
		
		if len(rulesToBeAdded) != 0:
			#XXX: this is a limitation of the universal node orchestrator, which does not allow to deploy a
			#	  VNF without flows involving such a VNF
			if not instantiateOnUniversalNode(rulesToBeAdded,vnfsToBeAdded, endpoints):	#Sends the new VNFs and flow rules to the universal node orchestrator
				resp.status = falcon.HTTP_500
				return
				#pass
					
		if not removeFromUniversalNode(rulesToBeRemoved,vnfsToBeRemoved): #Save on a file the IDs of the rules and the NFs to be removed from the universal node
			resp.status = falcon.HTTP_500
			return
			#pass

		# 
		# The required modifications have been implemented in the universal node, then we can update the
		# configuration saved in the proper files
		#
		
		if not addToGraphFile(rulesToBeAdded,vnfsToBeAdded, endpoints): #Update the json representation of the deployed graph, by inserting the new VNFs/rules
			resp.status = falcon.HTTP_500
			LOG.error("Please, press 'ctrl+c' and restart the virtualizer.")
			LOG.error("Please, also restart the universal node orchestrator.")
			return
			
		if not removeFromGraphFile(vnfsToBeRemoved,rulesToBeRemoved): #Update the json representation of the deployed graph, by inserting the new VNFs/rules
			resp.status = falcon.HTTP_500
			LOG.error("Please, press 'ctrl+c' and restart the virtualizer.")
			LOG.error("Please, also restart the universal node orchestrator.")
			return
			
		if not updateUniversalNodeConfig(content): #Updates the file containing the current configuration of the universal node, by editing the #<flowtable> and the <NF_instances>
			resp.status = falcon.HTTP_500
			LOG.error("Please, press 'ctrl+c' and restart the virtualizer.")
			LOG.error("Please, also restart the universal node orchestrator.")
			return
		
		LOG.info("'edit-config' command properly handled")

def isCorrect(newContent):
	'''
	Check if the new configuration of the node (in particular, the flowtable) is correct:
	*	the ports are part of the universal node
	*	the VNFs referenced in the flows are instantiated
	'''
	
	LOG.debug("Checking the correctness of the new configuration...")

	LOG.debug("Reading file '%s', which contains the current configuration of the universal node...",constants.GRAPH_XML_FILE)
	try:
		oldTree = ET.parse(constants.GRAPH_XML_FILE)
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		return False
	LOG.debug("File correctly read")
		
	infrastructure = Virtualizer.parse(root=oldTree.getroot())
	universal_node = infrastructure.nodes.node[constants.NODE_ID]
	flowtable = universal_node.flowtable
	nfInstances = universal_node.NF_instances
	
	#tmpInfra = copy.deepcopy(infrastructure)
	
	LOG.debug("Getting the new flowrules to be installed on the universal node")
	try:
		newTree = ET.ElementTree(ET.fromstring(newContent))
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		return False
						
	newInfrastructure = Virtualizer.parse(root=newTree.getroot())
	newFlowtable = newInfrastructure.nodes.node[constants.NODE_ID].flowtable
	newNfInstances = newInfrastructure.nodes.node[constants.NODE_ID].NF_instances
							
	#Update the NF instances with the new NFs
	try:
		for instance in newNfInstances:
			if instance.get_operation() == 'delete':
				nfInstances[instance.id.get_value()].delete()
			else:
				nfInstances.add(instance)
	except KeyError:
		LOG.error("Trying to delete a VNF that does not exist! ID:%s", instance.id.get_value())
		return False
			
	#Update the flowtable with the new flowentries
	try:
		for flowentry in newFlowtable:
			if flowentry.get_operation() == 'delete':
				flowtable[flowentry.id.get_value()].delete()
			else:
				flowtable.add(flowentry) 
	except KeyError:
		LOG.error("Trying to delete a flowrule that does not exist! ID:%s", flowentry.id.get_value())
		return False
		

	#Here, infrastructure contains the new configuration of the node
	#Then, we execute the checks on it!
	
	#TODO

	LOG.debug("The new configuration of the universal node is correct!")
		
	return True
	
def extractVNFsInstantiated(content):
	'''
	Parses the message and extracts the type of the deployed network functions.
	
	As far as I understand, the 'type' in a NF is the linker between <NF_instances>
	and <capabilities><supported_NFs>. Then, this function also checks that the type
	of the NF to be instantiated is among those to be supported by the universal node
	'''
	
	global error

	try:
		tree = ET.parse(constants.GRAPH_XML_FILE)
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.server
		return
	
	tmpInfrastructure = Virtualizer.parse(root=tree.getroot())
	supportedNFs = tmpInfrastructure.nodes.node[constants.NODE_ID].capabilities.supported_NFs
	supportedTypes = []
	for nf in supportedNFs:
		nfType = nf.type.get_value()
		supportedTypes.append(nfType)
	
	LOG.debug("Extracting the network functions (to be) deployed on the universal node")
	try:
		tree = ET.ElementTree(ET.fromstring(content))
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.client
		return
	
	infrastructure = Virtualizer.parse(root=tree.getroot())
	universal_node = infrastructure.nodes.node[constants.NODE_ID]
	instances = universal_node.NF_instances	
	
	foundTypes = []
	nfinstances = []
	
	LOG.debug("Considering instances:")
	LOG.debug("'%s'",infrastructure.xml())
	
	for instance in instances:
		if instance.get_operation() == 'delete':
			#This network function has to be removed from the universal node
			continue
			
		vnfType = instance.type.get_value()
		if vnfType not in supportedTypes:
			LOG.warning("VNF of type '%s' is not supported by the UN!",vnfType)
			error = Error.client
			return
		
		if vnfType in foundTypes:
			LOG.warning("Found multiple NF instances with the same type '%s'!",vnfType)
			LOG.warning("This is not supported by the universal node!")
			error = Error.client
			return
			
		foundTypes.append(vnfType)
		port_list = []
		for port in instance.ports.port:
			port_list.append(Port(_id="port:"+port))
			
		vnf = VNF(_id = instance.id.get_value(), name = vnfType, ports=port_list)
		nfinstances.append(vnf)
		LOG.debug("Required VNF: '%s'",vnfType)
		
	return nfinstances

def extractRules(content):
	'''
	Parses the message and translates the flowrules in the internal JSON representation
	Returns a json representing the rules in the internal format of the universal node
	'''
	
	global error
	
	LOG.debug("Extracting the flowrules to be installed in the universal node")
	
	try:
		tree = ET.ElementTree(ET.fromstring(content))
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.client
		return
	
	infrastructure = Virtualizer.parse(root=tree.getroot())
	universal_node = infrastructure.nodes.node[constants.NODE_ID]
	flowtable = universal_node.flowtable
	
	endpoints_dict = {}
	endpoint_id = 1
		
	flowrules = []
	for flowentry in flowtable:		
		if flowentry.get_operation() == 'delete':
			#This rule has to be removed from the universal node
			continue
	
		flowrule = FlowRule()
		
		f_id = flowentry.id.get_value()
		priority = flowentry.priority.get_value()
		
		#Iterate on the match in order to translate it into the json syntax
		#supported internally by the universal node
		#match = {}
		match = Match() 
		if flowentry.match is not None:
			if type(flowentry.match.get_value()) is str:
				#The tag <match> contains a sequence of matches separated by " "
				matches = flowentry.match.data.split(" ")
				for m in matches:
					tokens = m.split("=")
					elements = len(tokens)
					if elements != 2:
						error = Error.client
						return 
					#The match is in the form "name=value"
					if not supportedMatch(tokens[0]):
						error = Error.client
						return
					#We have to convert the virtualizer match into the UN equivalent match
					
					setattr(match, equivalentMatch(tokens[0]), tokens[1])

			#We ignore the element in case it's not a string. It is possible that it is simply empty
					
		#The content of <port> must be added to the match
		#XXX: the following code is quite dirty, but it is a consequence of the nffg library

		portPath = flowentry.port.get_target().get_path()
		port = flowentry.port.get_target()	
		tokens = portPath.split('/');
						
		if len(tokens) is not 6 and len(tokens) is not 8:
			LOG.error("Invalid port '%s' defined in a flowentry (len(tokens) returned %d)",portPath,len(tokens))
			error = Error.client
			return
						
		if tokens[4] == 'ports':
			#This is a port of the universal node. We have to extract the virtualized port name
			port_name = physicalPortsVirtualization[port.name.get_value()]		
			if port_name not in endpoints_dict:
				endpoints_dict[port_name] = EndPoint(_id = str(endpoint_id) ,_type = "interface", interface = port_name)
				endpoint_id = endpoint_id + 1
			match.port_in = "endpoint:" + endpoints_dict[port_name].id
		elif tokens[4] == 'NF_instances':
			#This is a port of the NF. I have to extract the port ID and the type of the NF.
			#XXX I'm using the port ID as name of the port
			vnf = port.get_parent().get_parent()
			vnf_id = vnf.id.get_value()
			port_id = port.id.get_value()
			match.port_in = "vnf:"+ vnf_id + ":port:" + port_id
		else:
			LOG.error("Invalid port '%s' defined in a flowentry",port)
			error = Error.client
			return
				
		action = Action()
		if flowentry.action is not None:
			if type(flowentry.action.data) is str:
				#The tag <action> contains a sequence of actions separated by " "
				actions = flowentry.action.data.split(" ")
				for a in actions:
					tokens = a.split(":")
					elements = len(tokens)
					if not supportedAction(tokens[0],elements-1):
						error = Error.client
						print "Returning because action is not supported"
						return
					if elements == 1:
						setattr(action, equivalentAction(tokens[0]), True)
					else:
						setattr(action, equivalentAction(tokens[0]), tokens[1])

			# We ignore the element in case it's not a string. It could be simply empty.
							
		#The content of <out> must be added to the action
		#XXX: the following code is quite dirty, but it is a consequence of the nffg library

		portPath = flowentry.out.get_target().get_path()
		port = flowentry.out.get_target()	
		tokens = portPath.split('/');
		if len(tokens) is not 6 and len(tokens) is not 8:
			LOG.error("Invalid port '%s' defined in a flowentry",portPath)
			error = Error.client
			return
					
		if tokens[4] == 'ports':
			#This is a port of the universal node. We have to extract the ID
			#Then, I have to retrieve the virtualized port name, and from there
			#the real name of the port on the universal node
			port_name = physicalPortsVirtualization[port.name.get_value()]		
			if port_name not in endpoints_dict:
				endpoints_dict[port_name] = EndPoint(_id = str(endpoint_id) ,_type = "interface", interface = port_name)
				endpoint_id = endpoint_id + 1
			action.output = "endpoint:" + endpoints_dict[port_name].id
		elif tokens[4] == 'NF_instances':
			#This is a port of the NF. I have to extract the port ID and the type of the NF.
			#XXX I'm using the port ID as name of the port			
			vnf = port.get_parent().get_parent()
			vnf_id = vnf.id.get_value()
			port_id = port.id.get_value()
			action.output = "vnf:" + vnf_id + ":port:" + port_id
		else:
			LOG.error("Invalid port '%s' defined in a flowentry",port)
			error = Error.client
			return

		#Prepare the rule
		flowrule.id = f_id
		if priority is None:
			LOG.error("Flowrule '%s' must have a priority set", f_id)
			error = Error.client
			return
		flowrule.priority = int(priority)
		flowrule.match = match
		flowrule.actions.append(action)
		
				
		flowrules.append(flowrule)
			
	LOG.debug("Rules extracted:")
	for rule in flowrules:
		LOG.debug(rule.getDict())
	
	return flowrules, endpoints_dict.values()
	
def	extractToBeRemovedVNFs(content):
	'''
	Parses the message and identifies those network functions to be removed
	
	The network functions to be removed must already be instantiated on the universal node. The
	type is used as a unique identifier for the network function.
	'''
	
	global error
	
	try:
		tree = ET.parse(constants.GRAPH_XML_FILE)
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.server
		return
	
	tmpInfrastructure = Virtualizer.parse(root=tree.getroot())
	nf_instances = tmpInfrastructure.nodes.node[constants.NODE_ID].NF_instances
	
	vnfsDeployed = []
	for vnf in nf_instances:
		ftype = vnf.type.get_value()
		vnfsDeployed.append(ftype)
		
	LOG.debug("Identifying the network functions to be removed from the universal node")
		
	try:
		tree = ET.ElementTree(ET.fromstring(content))
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.client
		return
	
	infrastructure = Virtualizer.parse(root=tree.getroot())
	universal_node = infrastructure.nodes.node[constants.NODE_ID]
	instances = universal_node.NF_instances	
	
	nfinstances = []
	for instance in instances:
		if instance.get_operation() == 'delete':
			vnfType = instance.type.get_value()
			if vnfType not in vnfsDeployed:
				LOG.warning("Network function with type '%s' is not deployed in the UN!",vnfType)
				LOG.warning("The network function cannot be removed!")
				error = Error.client
				return
			
			LOG.debug("Network function with type '%s' has to be removed",vnfType)
			nfinstances.append(vnfType)
	
	return nfinstances	

def extractToBeRemovedRules(content):
	'''
	Parses the message and identifies those flowrules to be removed.
	
	The rules to be removed must be already instantiated on the universal node. The rule ID
	is used as a unique identifier for the rules.
	'''

	global error

	try:
		tree = ET.parse(constants.GRAPH_XML_FILE)
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.server
		return
	
	tmpInfrastructure = Virtualizer.parse(root=tree.getroot())
	flowtable = tmpInfrastructure.nodes.node[constants.NODE_ID].flowtable
	rulesDeployed = []
	for flowrule in flowtable:
		fid = flowrule.id.get_value()
		rulesDeployed.append(fid)

	LOG.debug("Identifying the flowrules to be removed from the universal node")
	
	try:
		tree = ET.ElementTree(ET.fromstring(content))
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		error = Error.client
		return
			
	infrastructure = Virtualizer.parse(root=tree.getroot())
	universal_node = infrastructure.nodes.node[constants.NODE_ID]
	flowtable = universal_node.flowtable
	
	ids = []
	for flowentry in flowtable:
		if flowentry.get_operation() == 'delete':
			f_id = flowentry.id.get_value()
			if f_id not in rulesDeployed:
				LOG.warning("Rule with ID '%d' is not deployed in the UN!",int(f_id))
				LOG.warning("The rule cannot be removed!")
				error = Error.client
				return
						
			LOG.debug("Rule with id %d has to be removed",int(f_id))
			ids.append(f_id)

	return ids
	
def diffRulesToBeAdded(newRules):
	'''
	Read the graph currently deployed. It is stored in a tmp file, in a json format.
	Then, compare it with the new request, in order to identify the new rules to be
	deployed.
	
	This function is useless in case the config coming from the network is a diff wrt
	the current configuration of the universal node.
	However, I let it here just in case sometimes the configuration received is not
	a diff.
	'''
	
	#FIXME: why don't just compare the IDs?
	
	global error	
		
	LOG.debug("Compare the new rules received with those already deployed")
	
	try:
		LOG.debug("Reading file: %s",constants.GRAPH_FILE)
		tmpFile = open(constants.GRAPH_FILE,"r")
		json_file = tmpFile.read()
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		error = Error.server
		return
	
	nffg_dict = json.loads(json_file)
	nffg = NF_FG()
	nffg.parseDict(nffg_dict)
	
	rulesToBeAdded = []
	
	for newRule in newRules:
		#For each new rule, compare it with the ones already part of the graph
		equal = False
		for oldrule in nffg.flow_rules:
			#if newRule.getDict() == oldrule.getDict():
			if newRule.id == oldrule.id:
				equal = True
				break
		
		if not equal:
			#The new rule is not yet part of the graph
			LOG.debug("Rule that must be inserted: ")
			LOG.debug("%s",json.dumps(newRule.getDict()))
			rulesToBeAdded.append(newRule)
			
	return rulesToBeAdded
		
def supportedMatch(tag):
	'''
	Given an element within match, this function checks whether such an element is supported or node
	'''
	
	if tag in constants.supported_matches:
		LOG.debug("'%s' is supported!",tag)
		return True
	else:
		LOG.error("'%s' is not a supported match!",tag)
		return False
		
def equivalentMatch(tag):
	'''
	Given an element within match, this function return the element with equivalent meaning in native orchestrator NF-FG
	'''
	return constants.supported_matches[tag]
	
def supportedAction(tag,elements):
	'''
	Given an element within an action, this function checks whether such an element is supported or not
	'''
	
	if tag in constants.supported_actions:
		LOG.debug("'%s' is supported with %d elements!",tag,constants.supported_actions[tag])
		if constants.supported_actions[tag] == elements:
			return True
		else:
			LOG.debug("The action specifies has a wrong number of elements: %d",elements)
			return False
	else:
		LOG.error("'%s' is not a supported action!",tag)
		return False
		
def equivalentAction(tag):
	'''
	Given an element within action, this function return the element with equivalent meaning in native orchestrator NF-FG
	'''
	return constants.equivalent_actions[tag]

def addToGraphFile(newRules,newVNFs, newEndpoints):
	'''
	Read the graph currently deployed. It is stored in a tmp file, in a json format.
	Then, adds to it the new VNFs, the new flowrules and the new endpoints to be instantiated.
	'''
	
	LOG.debug("Updating the json representation of the whole graph deployed")

	try:
		LOG.debug("Reading file: %s",constants.GRAPH_FILE)
		tmpFile = open(constants.GRAPH_FILE,"r")
		json_file = tmpFile.read()
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
	
	nffg_dict = json.loads(json_file)
	nffg = NF_FG()
	nffg.parseDict(nffg_dict)
				
	#Add the new flowrules
	for nr in newRules:
		nffg.addFlowRule(nr)
	
	#Add the new VNFs
	for vnf in newVNFs:
		if nffg.getVNF(vnf.id) is None:
			LOG.debug("New VNF: %s!",vnf.name)
			LOG.debug("The VNF must be inserted!")
			nffg.addVNF(vnf)
	
	#Add the new Endpoints
	for endp in newEndpoints:
		already_present = False
		for endpoint in nffg.end_points:
			if endp.interface == endpoint.interface:
				already_present = True
				break
		if already_present is False:		
			nffg.addEndPoint(endp)
	
	LOG.debug("Updated graph:");	
	LOG.debug("%s",nffg.getJSON());
	
	try:
		tmpFile = open(constants.GRAPH_FILE, "w")
		tmpFile.write(nffg.getJSON())
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		error = True
		return False
		
	return True
	
def removeFromGraphFile(vnfsToBeRemoved,rulesToBeRemoved):
	'''
	Read the graph currently deployed. It is stored in a tmp file, in a json format.
	Then, removes from it the VNFs and the flowrules to be removed
	'''
	
	LOG.debug("Removing VNFs and flowrules from the graph containing the json representation of the graph")
	
	try:
		LOG.debug("Reading file: %s",constants.GRAPH_FILE)
		tmpFile = open(constants.GRAPH_FILE,"r")
		json_file = tmpFile.read()
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
	
	nffg_dict = json.loads(json_file)
	nffg = NF_FG()
	nffg.parseDict(nffg_dict)

	for vnf in nffg.vnfs[:]:
		if vnf.name in vnfsToBeRemoved:
			nffg.vnfs.remove(vnf)
	
	for rule in nffg.flow_rules[:]:
		if rule.id in rulesToBeRemoved:
			nffg.flow_rules.remove(rule)
	
	for endpoint in nffg.end_points[:]:
		if not nffg.getFlowRulesSendingTrafficToEndPoint(endpoint.id) and not nffg.getFlowRulesSendingTrafficFromEndPoint(endpoint.id):
			nffg.end_points.remove(endpoint)
	
	LOG.debug("Updated graph:");	
	LOG.debug("%s",nffg.getJSON());
	
	try:
		tmpFile = open(constants.GRAPH_FILE, "w")
		tmpFile.write(nffg.getJSON())
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
		
	return True

def updateUniversalNodeConfig(newContent):
	'''
	Read the configuration of the universal node, and applies the required modifications to
	the NF instances and to the flowtable
	'''
	
	LOG.debug("Updating the file containing the configuration of the node...")
	
	LOG.debug("Reading file '%s', which contains the current configuration of the universal node...",constants.GRAPH_XML_FILE)
	try:
		oldTree = ET.parse(constants.GRAPH_XML_FILE)
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		return False
	LOG.debug("File correctly read")
		
	infrastructure = Virtualizer.parse(root=oldTree.getroot())
	universal_node = infrastructure.nodes.node[constants.NODE_ID]
	flowtable = universal_node.flowtable
	nfInstances = universal_node.NF_instances
	
	tmpInfra = copy.deepcopy(infrastructure)
	
	LOG.debug("Getting the new flowrules to be installed on the universal node")
	try:
		newTree = ET.ElementTree(ET.fromstring(newContent))
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		return False
			
	newInfrastructure = Virtualizer.parse(root=newTree.getroot())
	newFlowtable = newInfrastructure.nodes.node[constants.NODE_ID].flowtable
	newNfInstances = newInfrastructure.nodes.node[constants.NODE_ID].NF_instances
			
	#Update the NF instances with the new NFs
	for instance in newNfInstances:
		if instance.get_operation() == 'delete':
			nfInstances[instance.id.get_value()].delete()
		else:
			nfInstances.add(instance)
	
	#Update the flowtable with the new flowentries
	for flowentry in newFlowtable:
		if flowentry.get_operation() == 'delete':
			flowtable[flowentry.id.get_value()].delete()
		else:
			flowtable.add(flowentry)
	#It is not necessary to remove conflicts, since they are already handled by the library,
	#i.e., it does not insert two identical rules
	
	try:
		tmpFile = open(constants.GRAPH_XML_FILE, "w")
		tmpFile.write(infrastructure.xml())
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
		
	return True


'''
	Methods used to interact with the universal node orchestrator
'''
def instantiateOnUniversalNode(rulesToBeAdded,vnfsToBeAdded, endpoints):
	'''
	Deploys rules and VNFs on the universal node
	'''
	LOG.info("Sending the new configuration to the universal node orchestrator (%s)",unOrchestratorURL)

	nffg = NF_FG()
	nffg.id = graph_id
	nffg.name = graph_name
	nffg.flow_rules = rulesToBeAdded
	nffg.vnfs = vnfsToBeAdded
	nffg.end_points = endpoints
	
	#Delete endpoints that are not involved in any flowrule
	for endpoint in nffg.end_points[:]:
		if not nffg.getFlowRulesSendingTrafficToEndPoint(endpoint.id) and not nffg.getFlowRulesSendingTrafficFromEndPoint(endpoint.id):
			nffg.end_points.remove(endpoint)
			endpoints.remove(endpoint)
	
	LOG.debug("Graph that is going to be sent to the universal node orchestrator:")
	LOG.debug("%s",nffg.getJSON())
	
	put_url = unOrchestratorURL + "/NF-FG/%s"

	try:
		responseFromUN = requests.put(put_url % (nffg.id), nffg.getJSON())
	except (requests.ConnectionError) as e:
		LOG.error("Cannot contact the universal node orchestrator at '%s'",put_url % (nffg.id))
		return False

	LOG.debug("Status code received from the universal node orchestrator: %s",responseFromUN.status_code)

	if responseFromUN.status_code == 201:
		LOG.info("New VNFs and flows properly deployed on the universal node")	
	else:
		LOG.error("Something went wrong while deploying the new VNFs and flows on the universal node")	
		return False

	return True

def removeFromUniversalNode(rulesToBeRemoved,vnfsToBeRemoved):
	# TODO:
	'''
	Removes rules from the universal node
	'''
	
	if len(vnfsToBeRemoved) != 0:
		LOG.warning("Required to remove '%d' VNFs",len(vnfsToBeRemoved))
		LOG.warning("Due to implementation choices of the universal node orchestrator, such VNFs will be only removed if no flow-rule will refer to their ports")
	
	if len(rulesToBeRemoved) == 0:
		# No message is sent to the orchestrator
		return True
	
	LOG.info("Removing %d rules from the universal node",len(rulesToBeRemoved))
	LOG.warning("Due to implementation choices of the universal node orchestrator, the VNFs whose all ports will no longer used in any deployed flow will be removed (undeployed)")
	
	delete_url = unOrchestratorURL + "/NF-FG/%s/%s"
	for rule in rulesToBeRemoved:
		LOG.debug("Going to remove rule with ID: %s",rule)	
		try:
			responseFromUN = requests.delete(delete_url % (graph_id,rule))
		except (requests.ConnectionError) as e:
			LOG.error("Cannot contact the universal node orchestrator at '%s'",unOrchestratorURL)	
			return False

		LOG.debug("Status code: %s",responseFromUN.status_code)
		
		if responseFromUN.status_code == 204:
			LOG.info("Rule '%s' has been properly deleted",rule)
		else:
			LOG.error("Something went wrong while deploying the new VNFs and flows on the universal node")	
			return False
			
	return True

'''
	Methods used in the initialization phase of the virtualizer
'''

def virtualizerInit():
	'''
	The virtualizer maintains the state of the node in a tmp file.
	This function initializes such a file.
	'''
	
	LOG.info("Initializing the virtualizer...")
	
	
	if not readConfigurationFile():
		return False
	
	v = Virtualizer(id=constants.INFRASTRUCTURE_ID, name=constants.INFRASTRUCTURE_NAME)				
	v.nodes.add(
		Infra_node(
			id=constants.NODE_ID,
			name=constants.NODE_NAME,
			type=constants.NODE_TYPE,
			resources=Software_resource(
				cpu='0',
				mem='0',
				storage='0'
			)
		)
	)
	
	#Read information related to the infrastructure and add it to the
	#virtualizer representation
	LOG.debug("Reading file '%s'...",infrastructureFile)
	try:
		tree = ET.parse(infrastructureFile)
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		#resp.status = falcon.HTTP_500
		return False
	root = tree.getroot()

	resources = root.find('resources')
	cpu = resources.find('cpu')
	memory = resources.find('memory')
	storage = resources.find('storage')
	
	thecpu = cpu.attrib
	thememory = memory.attrib
	thestorage = storage.attrib
	
	LOG.debug("CPU: %s", thecpu['amount'])
	LOG.debug("memory: %s %s", thememory['amount'],thememory['unit'])
	LOG.debug("storage: %s %s", thestorage['amount'],thestorage['unit'])
	
	universal_node = v.nodes.node[constants.NODE_ID]
	resources = universal_node.resources
	resources.cpu.set_value(thecpu['amount'] + " VCPU")
	resources.mem.set_value(thememory['amount'] + " " + thememory['unit'])
	resources.storage.set_value(thestorage['amount'] + " " + thestorage['unit'])
	
	#Read information related to the physical ports and add it to the
	#virtualizer representation
	
	#global physicalPortsVirtualization

	ports = root.find('ports')
	portID = 1
	for port in ports:
		virtualized = port.find('virtualized')
		port_description = virtualized.attrib
		LOG.debug("physicl name: %s - virtualized name: %s - type: %s - sap: %s", port.attrib['name'], port_description['as'],port_description['port-type'],port_description['sap'])
		physicalPortsVirtualization[port_description['as']] =  port.attrib['name']

		portObject = Virt_Port(id=str(portID), name=port_description['as'], port_type=port_description['port-type'], sap=port_description['sap'])
		universal_node.ports.add(portObject)	
		portID = portID + 1
	
	#Save the virtualizer representation on a file
	try:
		tmpFile = open(constants.GRAPH_XML_FILE, "w")
		tmpFile.write(v.xml())
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
	
	if not contactNameResolver():
		return False
	
	#Initizialize the file describing the deployed graph as a json
	flowRules=[]
	vnfs=[]
	endpoints = []
	if not toBeAddedToFile(flowRules,vnfs,endpoints,constants.GRAPH_FILE):
		return False
	
	LOG.info("The virtualizer has been initialized")
	return True

def readConfigurationFile():
	'''
	Read the configuration file of the virtualizer
	'''
	
	global nameResolverURL
	global unOrchestratorIP
	global unOrchestratorURL
	global infrastructureFile
	
	LOG.info("Reading configuration file: '%s'",constants.CONFIGURATION_FILE)
	config = ConfigParser.ConfigParser()
	config.read(constants.CONFIGURATION_FILE)
	sections = config.sections()
	
	if 'connections' not in sections:
		LOG.error("Wrong file '%s'. It does not include the section 'connections' :(",constants.CONFIGURATION_FILE)
		return False
	try:
		nameResolverURL = nameResolverURL + config.get("connections","NameResolverAddress") + ":" + config.get("connections","NameResolverPort")
	except:
		LOG.error("Option 'NameResolverAddress' or option 'NameResolverPort' not found in section 'connections' of file '%s'",constants.CONFIGURATION_FILE)
		return False
	try:
		unOrchestratorIP = config.get("connections","UNOrchestratorAddress")
		unOrchestratorURL = unOrchestratorURL + unOrchestratorIP + ":" + config.get("connections","UNOrchestratorPort")
	except:
		LOG.error("Option 'UNOrchestratorAddress' or option 'UNOrchestratorPort' not found in section 'connections' of file '%s'",constants.CONFIGURATION_FILE)
		return False
	
	if 'configuration' not in sections:
		LOG.error("Wrong file '%s'. It does not include the section 'configuration' :(",constants.CONFIGURATION_FILE)
		return False
	try:
		infrastructureFile = config.get("configuration","UNOrchestratorConfigFile")
	except:
		LOG.error("Option 'UNOrchestratorConfigFile' not found in section 'configuration' of file '%s'",constants.CONFIGURATION_FILE)
		return False	
	try:
		LogLevel = config.get("configuration","LogLevel")	
		if LogLevel == 'debug':
			LOG.setLevel(logging.DEBUG)
			LOG.addHandler(sh)
			LOG.debug("Log level set to 'debug'")
		if LogLevel == 'info':
			LOG.setLevel(logging.INFO)
			LOG.info("Log level set to 'info'")
		if LogLevel == 'warning':
			LOG.setLevel(logging.WARNING)
			LOG.warning("Log level set to 'warning'")
		if LogLevel == 'error':
			LOG.setLevel(logging.ERROR)
			LOG.error("Log level set to 'error'")
		if LogLevel == 'critical':
			LOG.setLevel(logging.CRITICAL)
			LOG.critical("Log level set to 'critical'")
	except:
		LOG.warning("Option 'LogLevel' not found in section 'configuration' of file '%s'",constants.CONFIGURATION_FILE)
		LOG.warning("Log level is set on 'INFO'")
	
	LOG.info("Url used to contact the name-resolver: %s",nameResolverURL)
	LOG.info("Url used to contact the universal node orchestrator: %s",unOrchestratorURL)
	
	return True

def contactNameResolver():
	'''
	Contact the name resolver is order to know the VNFs available
	'''
	
	LOG.info("Starting interaction with the name-resolver (%s)",nameResolverURL)
	
	url = nameResolverURL + "/nfs/digest"
	try:
		response = requests.get(url)
	except (requests.ConnectionError) as e:
		LOG.error("Cannot contact the name-resolver at %s",url)
		return False
	
	data = response.json()
	
	LOG.debug("Data received from the name-resolver")
	LOG.debug("%s",json.dumps(data, indent = 4))
	
	json_object = data
	
	if 'network-functions' not in json_object.keys():
		LOG.error("Wrong response received from the 'name-resolver'")
		return False
	
	sequence_number = 1
	for vnf_name in json_object['network-functions']:
		if 'name' not in vnf_name:
			LOG.error("Wrong response received from the 'name-resolver'")
			return False
		LOG.debug("Considering VNF: '%s'",vnf_name['name'])
		
		url = nameResolverURL + '/nfs/' + vnf_name['name']
		try:
			response = requests.get(url)
		except (requests.ConnectionError) as e:
			LOG.error("Cannot contact the name-resolver at %s",url)
			return False
	
		vnf_description = response.json()
	
		LOG.debug("Data received from the name-resolver")
		LOG.debug("%s",json.dumps(vnf_description, indent = 4))
		
		if 'name' not in vnf_description:
			LOG.error("Wrong response received from the 'name-resolver'")
			return False

		if 'nports' not in vnf_description:
			LOG.error("Wrong response received from the 'name-resolver'")
			return False
		
		if 'summary' not in vnf_description:
			LOG.error("Wrong response received from the 'name-resolver'")
			return False
		
		ID = 'NF'+str(sequence_number)
		name = vnf_description['summary']
		vnftype = vnf_description['name']
		numports = vnf_description['nports']
		
		try:
			tree = ET.parse(constants.GRAPH_XML_FILE)
		except ET.ParseError as e:
			print('ParseError: %s' % e.message)
			return False
	
		LOG.debug("Inserting VNF %s, ID %s, type %s, num ports %d...",ID,name,vnftype,numports)
	
		infrastructure = Virtualizer.parse(root=tree.getroot())
		universal_node = infrastructure.nodes.node[constants.NODE_ID]
		capabilities = universal_node.capabilities
		supportedNF = capabilities.supported_NFs
	
		vnf = Infra_node(id=ID,name=name,type=vnftype)
	
		i = 1
		for x in range(0, numports):
			port = Virt_Port(id=str(i), name='VNF port ' + str(i), port_type='port-abstract')
			vnf.ports.add(port)
			i = i+1
	
		supportedNF.add(vnf)
	
		try:
			tmpFile = open(constants.GRAPH_XML_FILE, "w")
			tmpFile.write(infrastructure.xml())
			tmpFile.close()
		except IOError as e:
			print "I/O error({0}): {1}".format(e.errno, e.strerror)
			return False

		sequence_number = sequence_number + 1
		LOG.debug("VNF '%s' considered",vnf_name['name'])
	
	LOG.info("Interaction with the name-resolver terminated")
	return True

def toBeAddedToFile(flowRules,vnfs,endpoints,fileName):
	'''
	Given a set (potentially empty) of flow rules and NFs, write it in a file respecting the syntax expected by the Univeral Node
	'''
	
	LOG.debug("Writing rules on file '%s'",fileName)
	
	nffg = NF_FG()
	nffg.flow_rules = flowRules
	nffg.vnfs = vnfs
	nffg.end_points = endpoints
	nffg.id = graph_id
	nffg.name = graph_name

	
	try:
		tmpFile = open(fileName, "w")
		tmpFile.write(nffg.getJSON())
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
		
	return True

'''
	The following code is executed by guicorn at the boot of the virtualizer
'''
	
api = falcon.API()

#Set the logger
LOG = logging.getLogger(__name__)
LOG.setLevel(logging.INFO)
LOG.propagate = False
sh = logging.StreamHandler()
f = logging.Formatter('[%(asctime)s][Virtualizer][%(levelname)s] %(message)s')
sh.setFormatter(f)
LOG.addHandler(sh)

#Global variables
unOrchestratorURL = "http://"
nameResolverURL = "http://"
infrastructureFile = ""
physicalPortsVirtualization = {}
graph_id = "1"
graph_name = "NF-FG"

if not virtualizerInit():
	LOG.error("Failed to start up the virtualizer.")
	LOG.error("Please, press 'ctrl+c' and restart the virtualizer.")

api.add_route('/',DoUsage())
api.add_route('/ping',DoPing())
api.add_route('/get-config',DoGetConfig())
api.add_route('/edit-config',DoEditConfig())

#in_file = open ("config/nffg_examples/passthrough_with_vnf_nffg_and_match_and_action.xml")
#in_file = open ("config/nffg_examples/nffg_delete_flow_vnf.xml")
#DoEditConfig().on_post(in_file.read(), None)

