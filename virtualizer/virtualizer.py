#!/usr/bin/env python

__author__ = 'Ivano Cerrato'

#sudo apt-get install python-pip
#sudo pip install gunicorn
#sudo pip install falcon
#sudo pip install cython

#gunicorn -b ip:port example:api

import sys
import falcon
import json
import logging
import os
import xml.etree.ElementTree as ET
import re
import xml.dom.minidom
import copy
import requests

import constants
from virtualizer_library.virtualizer3 import *

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
		b = 'get http://hostip:tcpport - this help message\n'
		c = 'get http://hostip:tcpport/ping - test webserver aliveness\n'
		d = 'post http://hostip:tcpport/get-config - query NF-FG\n'
		e = 'post http://hostip:tcpport/edit-config - send NF-FG request in the post body\n'
		f = '\n'
		g = 'limitations:\n'
		h = 'the flowrule ID must be unique on the node.\n'
		i = 'type cannot be repeated in multiple NF instances.\n'
		j = 'capabilities are not supported.\n'
		k = 'actions are not supported.\n'	
		self._answer = a + b + c + d + e + f + g + h + i + j + k

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
		LOG.debug("Reading file: %s",constants.CONFIGURATION_FILE)

		try:
			tree = ET.parse(constants.CONFIGURATION_FILE)
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
		
		LOG.debug("Body of the request:")
		LOG.debug("%s",content)
		
		#TODO: check that flows refer to existing (i.e., deployed) network function.
		#TODO: check that flows refer to existing ports 
		
		#FIXME: not clear what this function actually does
		if not isCorrect(content):
			resp.status = falcon.HTTP_400
			return	
			
		#
		#	Extract the needed information from the message received from the network
		#
	
#		vnfsToBeAdded = extractVNFsInstantiated(content)	#VNF deployed/to be deployed on the universal node
#		if error:
#			resp.status = falcon.HTTP_400
#			return	
		
		LOG.info("'edit-config' command properly handled")

def isCorrect(newContent):
	'''
	Check if the new configuration of the node (in particular, the flowtable) is correct:
	*	the ports are part of the universal node
	*	the VNFs referenced in the flows are instantiated
	'''
	
	LOG.debug("Checking the correctness of the new configuration...")

	LOG.debug("Reading file '%s', which contains the current configuration of the universal node...",constants.CONFIGURATION_FILE)
	try:
		oldTree = ET.parse(constants.CONFIGURATION_FILE)
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

	#Here, infrastructure contains the new configuration of the node
	#Then, we execute the checks on it!
	
	#TODO

	LOG.debug("The new configuration of the universal node is correct!")
		
	return True


'''
	Methods not belonging to any class
'''

def virtualizerInit():
	'''
	The virtualizer maintains the state of the node in a tmp file.
	This function initializes such a file.
	'''
	
	LOG.info("Initializing the virtualizer...")
	
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
	try:
		tree = ET.parse('infrastructure.xml')
	except ET.ParseError as e:
		print('ParseError: %s' % e.message)
		resp.status = falcon.HTTP_500
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
	ports = root.find('ports')
	portID = 1
	for port in ports:
		virtualized = port.find('virtualized')
		port_description = virtualized.attrib
		LOG.debug("Name: %s - type: %s - sap: %s", port_description['as'],port_description['port-type'],port_description['sap'])
		port = Port(id=str(portID), name=port_description['as'], port_type=port_description['port-type'], sap=port_description['sap'])
		universal_node.ports.add(port)
		portID = portID + 1
	
	#Save the virtualizer representation on a file
	try:
		tmpFile = open(constants.CONFIGURATION_FILE, "w")
		tmpFile.write(v.xml())
		tmpFile.close()
	except IOError as e:
		print "I/O error({0}): {1}".format(e.errno, e.strerror)
		return False
	
	if not contactNameResolver():
		return False
			
	LOG.info("The virtualizer has been initialized")
	return True

def contactNameResolver():
	'''
	Contact the name resolver is order to know the VNFs available
	'''
	
	LOG.info("Starting interaction with the name-resolver")
	
	response = requests.get('http://127.0.0.1:2626/nfs/digest')
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
		
		url = 'http://127.0.0.1:2626/nfs/' + vnf_name['name']
		response = requests.get(url)
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
			tree = ET.parse(constants.CONFIGURATION_FILE)
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
			port = Port(id=str(i), name='VNF port ' + str(i), port_type='port-abstract')
			vnf.ports.add(port)
			i = i+1
	
		supportedNF.add(vnf)
	
		try:
			tmpFile = open(constants.CONFIGURATION_FILE, "w")
			tmpFile.write(infrastructure.xml())
			tmpFile.close()
		except IOError as e:
			print "I/O error({0}): {1}".format(e.errno, e.strerror)
			return False

		sequence_number = sequence_number + 1
		LOG.debug("VNF '%s' considered",vnf_name['name'])
	
	LOG.info("Interaction with the name-resolver terminated")
	return True

'''
	The following code is executed by guicorn at the boot of the virtualizer
'''
	
api = falcon.API()

#Set the logger
LOG = logging.getLogger(__name__)
LOG.setLevel(logging.DEBUG)	#Change here the logging level
LOG.propagate = False
sh = logging.StreamHandler()
sh.setLevel(logging.DEBUG)
f = logging.Formatter('[%(asctime)s][Virtualizer][%(levelname)s] %(message)s')
sh.setFormatter(f)
LOG.addHandler(sh)

if not virtualizerInit():
	LOG.error("Failed to start up the virtualizer.")
	LOG.error("Please, press 'ctrl+c' and restart the virtualizer.")

error = False

api.add_route('/',DoUsage())
api.add_route('/ping',DoPing())
api.add_route('/get-config',DoGetConfig())
api.add_route('/edit-config',DoEditConfig())

