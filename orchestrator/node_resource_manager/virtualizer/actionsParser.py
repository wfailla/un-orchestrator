#!/usr/bin/env python
__author__ = 'Ivano Cerrato'

#Constants used by the parser
import constants

def handleSpecificAction(tokens):
	'''
	Given an action, invokes the proper handler for such an action
	'''	
	
	if tokens[0] == "strip_vlan":
		return handleStripVlanAction(tokens)
		
	if tokens[0] == "push_vlan":
		return handlePushVlanAction(tokens)
	
	#XXX add here code to handle further actions
	
	#Cannot be here
	
	return ""

def handleStripVlanAction(node):
	'''
	Parses the content of an action and translates it in the internal JSON representation.
	Such a representation is then returned.
	'''
	
	action = {}
	action["operation"] = "pop"
		
	return action
	
def handlePushVlanAction(node):
	'''
	Parses the content of an action and translates it in the internal JSON representation.
	Such a representation is then returned.
	'''
	
	action = {}

	action["operation"] = "push"
	action["vlan_id"] = tokens[1]
		
	return action
