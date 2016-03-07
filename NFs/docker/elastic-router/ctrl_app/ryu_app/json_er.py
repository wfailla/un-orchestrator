
from nffg_library.nffg import *
from er_utils import *
import json

import logging
import xml.etree.ElementTree as ET
import threading

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

                flowrules = nffg.getFlowRulesSendingTrafficToPort(ovsId, portId)
                for flowrule in flowrules:
                    port_in = flowrule.match.port_in
                    tokens = port_in.split(':')
                    port_in_type = tokens[0]
                    port_in_element_id = tokens[1]
                    port_in_id = ':'.join(tokens[2:])
                    #check if external port
                    if port_in_type == 'endpoint':
                        SAP_name =  nffg.getEndPoint(port_in_element_id).name
                        new_port.port_type = DPPort.External
                        linked_port = DPPort(SAP_name, port_in_id, port_type=DPPort.SAP)
                    #check if internal port
                    elif port_in_type == 'vnf':
                        VNF_name =  nffg.getVNF(port_in_element_id).name
                        if not 'ovs' in VNF_name:
                            continue
                        new_port.port_type = DPPort.Internal
                        linked_port = DPPort(VNF_name, port_in_id, port_type=DPPort.Internal)

                    new_port.linked_port = linked_port
                    linked_port.linked_port = new_port

    return ovs_instances

def add_vnf(nffg_json, id, name, vnftype, numports):
    json_dict = json.loads(nffg_json)
    nffg = NF_FG()
    nffg.parseDict(json_dict)

    #new_ovs = VNF()

    return nffg.getJSON()


if __name__ == "__main__":
    json_file = open('er_nffg.json').read()
    DP_switches = process_nffg(json_file)
    print 'end'

