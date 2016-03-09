
from nffg_library.nffg import *
from er_utils import *
import json

import logging
import xml.etree.ElementTree as ET
import threading
import urllib2

#Set the logger
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

def process_nffg(nffg_json):

    logging.debug("Reading dict '%s'...", nffg_json)

    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    ovs_instances = {}
    for vnf in nffg.vnfs:
        vnf_name = vnf.name
        logging.debug("found NF: %s", vnf_name)
        if 'ovs' in vnf_name:
            ovsName = vnf_name
            ovsId = vnf.id
            logging.debug("found ovs NF: %s", ovsName)
            ovs_instances.setdefault(ovsName, {'id': 0, 'ports': []})
            for port in vnf.ports:
                # do not add control port of the DP
                if 'control' in port.name: continue

                portName = port.name
                portId = port.id
                new_port = DPPort(portName, portId)
                ovs_instances[ovsName]['ports'].append(new_port)
                ovs_instances[ovsName]['id'] = ovsId
                logging.debug("found ovs port: %s with ovs id: %s", portName, ovsId)

    # first make all the ovs instances with all th ports,
    # then fill the linked ports
    # this function is only used to parse the first nffg, external ports only
    for ovs_name in ovs_instances:
        ovsId = ovs_instances[ovs_name]['id']
        for port in ovs_instances[ovs_name]['ports']:
            portId = port.id
            flowrules = nffg.getFlowRulesSendingTrafficFromPort(ovsId, portId)
            for flowrule in flowrules:
                # assume only one action
                port_in = flowrule.actions[0].output
                tokens = port_in.split(':')
                port_in_type = tokens[0]
                port_in_element_id = tokens[1]
                port_in_id = ':'.join(tokens[2:])
                #check if external port
                if port_in_type == 'endpoint':
                    SAP_name =  nffg.getEndPoint(port_in_element_id).name
                    port.port_type = DPPort.External
                    linked_port = DPPort(SAP_name, port_in_id, port_type=DPPort.SAP)
                #check if internal port
                elif port_in_type == 'vnf':
                    vnf =  nffg.getVNF(port_in_element_id)
                    VNF_name = vnf.name
                    if not 'ovs' in VNF_name:
                        continue
                    port_in_name = vnf.getPort(port_in_id).name
                    port.port_type = DPPort.Internal
                    for port2 in ovs_instances[VNF_name]['ports']:
                        if port2.ifname == port_in_name:
                            linked_port = port2
                    #linked_port = DPPort(port_in_name, port_in_id, port_type=DPPort.Internal)

                port.linked_port = linked_port
                linked_port.linked_port = port

    return ovs_instances

def add_vnf(nffg_json, id, name, vnftype, numports):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    #new_ovs = VNF()

    return nffg.getJSON()

def get_next_flowrule_id(nffg_json):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    flow_id_list = []
    for flowrule in nffg.flow_rules:
        id = int(flowrule.id)
        flow_id_list.append(id)
    max_id = max(flow_id_list)
    next_id_str = str(max_id+1).zfill(9)

    return next_id_str

def get_next_vnf_id(nffg_json, add=0):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    vnf_id_list = []
    for vnf in nffg.vnfs:
        id = int(vnf.id)
        vnf_id_list.append(id)
    max_id = max(vnf_id_list)
    next_id_str = str(max_id+1+add).zfill(8)

    return next_id_str


def add_flowentry_SAP(nffg_json, port_in, port_out, priority=10):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    flow_id = get_next_flowrule_id(nffg_json)

    if port_in.port_type == DPPort.SAP and port_out.port_type == DPPort.External:
        match_endpoints = nffg.getEndPointsFromName(port_in.ifname)
        match_entry = 'endpoint:' + match_endpoints[0].id
        new_match = Match(port_in=match_entry)

        action_vnf = nffg.getVNF(port_out.DP.id)
        action_port_id = ''
        for port in action_vnf.ports:
            if port_out.ifname in port.name:
                action_port_id = port.id
                break
        action_entry = 'vnf:' + action_vnf.id + ':' + action_port_id
        new_action = Action(output=action_entry)

    elif port_in.port_type == DPPort.External and port_out.port_type == DPPort.SAP:
        match_vnf = nffg.getVNF(port_in.DP.id)
        match_port_id = ''
        for port in match_vnf.ports:
            if port_in.ifname in port.name:
                match_port_id = port.id
                break
        match_entry = 'vnf:' + match_vnf.id + ':' + match_port_id
        new_match = Match(port_in=match_entry)

        action_endpoints = nffg.getEndPointsFromName(port_out.ifname)
        action_entry = 'endpoint:' + action_endpoints[0].id
        new_action = Action(output=action_entry)

    if 'new_match' in locals() and 'new_action' in locals():
        new_flow_rule = FlowRule(_id=flow_id, priority=priority, match=new_match, actions=[new_action])
        nffg.addFlowRule(new_flow_rule)

    return nffg.getJSON()

#can only be used for internal flow_rule, between ovs's
def add_flowentry(nffg_json, port_in, port_out, priority=10):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    flow_id = get_next_flowrule_id(nffg_json)

    match_vnf = nffg.getVNF(port_in.DP.id)
    logging.info('id of DP port_in: {0}'.format(port_in.DP.id))
    match_port_id = ''
    for port in match_vnf.ports:
        if port_in.ifname in port.name:
            match_port_id = port.id
            break
    match_entry = 'vnf:' + match_vnf.id + ':' + match_port_id
    new_match = Match(port_in=match_entry)

    action_vnf = nffg.getVNF(port_out.DP.id)
    action_port_id = ''
    for port in action_vnf.ports:
        if port_out.ifname in port.name:
            action_port_id = port.id
            break
    action_entry = 'vnf:' + action_vnf.id + ':' + action_port_id
    new_action = Action(output=action_entry)

    new_flow_rule = FlowRule(_id=flow_id, priority=priority, match=new_match, actions=[new_action])

    nffg.addFlowRule(new_flow_rule)

    return nffg.getJSON()

def delete_VNF(nffg_json, vnf_id, RESTaddress):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    del_vnf = nffg.getVNF(vnf_id)
    flows_out = nffg.getFlowRulesSendingTrafficFromVNF(del_vnf)
    flows_in = nffg.getFlowRulesSendingTrafficToVNF(del_vnf)

    flow_list = flows_out + flows_in

    for flow in flow_list:
        delete_flowrule(flow.id, RESTaddress)

def delete_flowrule(rule_id, RESTaddress):
    url = RESTaddress + '/NF-FG/NF-FG/' + rule_id
    req = urllib2.Request(url)
    req.get_method = lambda: 'DELETE'
    response = urllib2.urlopen(req)
    result = response.read()
    logging.info(result)

if __name__ == "__main__":
    json_file = open('er_nffg.json').read()
    DP_switches = process_nffg(json_file)

    flow_id = get_next_flowrule_id(json_file)
    print 'next flow id: {0}'.format(flow_id)

    #new_nffg = add_flowrule(json_file)

    print 'end'

