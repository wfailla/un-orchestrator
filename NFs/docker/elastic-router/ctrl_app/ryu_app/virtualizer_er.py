__author__ = 'Steven Van Rossem'

#NF-FG library
#import sys
#sys.path.append('virtualizer')
#import os, imp
#imp.load_source("virtualizer", os.path.join(os.path.dirname(__file__), "virtualizer/virtualizer3.py"))

from virtualizer.virtualizer3 import *
from er_utils import *
#import virtualizer3

import logging
import xml.etree.ElementTree as ET

#Set the logger
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

def process_nffg(nffg_xml):

    logging.debug("Reading tmp file '%s'...", nffg_xml)

    try:
        tree = ET.fromstring(nffg_xml)
    except ET.ParseError as e:
        print('ParseError: %s' % e.message)
        return 0
    logging.debug("File correctly read")

    nffg = Virtualizer.parse(root=tree)
    universal_nodes = nffg.nodes
    for un in universal_nodes:
        un_id = un.id.get_value()
        logging.debug("found UN: %s", un.id.get_value())
        ovs_switches = find_ovs(un)
        return ovs_switches


def find_ovs(un):
    """
    find ER data paths (ovs containers) in the topology
    :param un: Universal Node object from the NFFG
    :return:
    """
    ovs_instances = {}
    nf_instances = un.NF_instances
    for nf in nf_instances:
        nf_name = nf.name.get_value()
        nf_type = nf.type.get_value()
        logging.debug("found NF: %s", nf.name.get_value())
        if 'ovs' in nf_type:
            ovsName = nf.name.get_value()
            ovsId = nf.id.get_value()
            logging.debug("found ovs NF: %s", ovsName)
            ovs_instances.setdefault(ovsName, {'id': 0, 'ports': []})
            # get ports of ovs (eth0 is control port by convention, only need switch ports)
            # TODO meta data in nffg to define control port, switch port, port name in docker, switch name in docker
            for port in nf.ports:
                port_name = port.name.get_value()
                portName = port.name.get_value()
                new_port = DPPort(portName)
                ovs_instances[ovsName]['ports'].append(new_port)
                ovs_instances[ovsName]['id'] = ovsId
                logging.debug("found ovs port: %s with id: %s", portName, ovsId)

    # check if the ports are internal nffg links or external to SAPs
    #ovs_instances_with_link_types = {}
    port_types = extract_links(un)
    for ovs in ovs_instances:
        for port in ovs_instances[ovs]['ports']:
            portName = port.ifname
            try:
                port.link_type = port_types[ovs][portName]
            except:
                logging.info("{0} port: {1} not found in nffg flowtable".format(ovs, portName))

    return ovs_instances

def extract_links(un):
    """
    Find the links of the ER topology in the NFFG, to know which ports are connected to SAPs and which are not.
    This information is needed for the routing in the ER ctrl_app.
    code from virtualizer.py in un-orchestrator
    :param un: Universal Node object from the NFFG
    :return:
    """
    port_types = {}
    flowtable_ = un.flowtable
    for flowentry_ in flowtable_:
        portPath_ = flowentry_.port.get_target().get_path()
        port = flowentry_.port.get_target()
        tokens = portPath_.split('/')

        if len(tokens) is not 6 and len(tokens) is not 8:
            logging.error("Invalid port '%s' defined in a flowentry (len(tokens) returned %d)",portPath_,len(tokens))
            error = True
            return

        if tokens[4] == 'ports':
            #This is a port of the universal node. We have to extract the virtualized port name
            src_node = ('external', port.name.get_value())
        elif tokens[4] == 'NF_instances':
            #This is a port of the NF. I have to extract the port ID and the type of the NF.
            #XXX I'm using the port ID as name of the port
            vnf = port.get_parent().get_parent()
            vnfType = vnf.type.get_value()
            vnfName = vnf.name.get_value()
            portID = port.id.get_value()
            portName = port.name.get_value()
            src_node= ('internal', portName, vnfType, vnfName)
        else:
            logging.error("Invalid port '%s' defined in a flowentry",port)
            error = True
            return

        portPath_ = flowentry_.out.get_target().get_path()
        port = flowentry_.out.get_target()
        tokens = portPath_.split('/')
        if len(tokens) is not 6 and len(tokens) is not 8:
            logging.error("Invalid port '%s' defined in a flowentry",portPath_)
            error = True
            return

        if tokens[4] == 'ports':
            #This is a port of the universal node. We have to extract the ID
            #Then, I have to retrieve the virtualized port name, and from there
            #the real name of the port on the universal node
            dst_node= ('external', port.name.get_value())
        elif tokens[4] == 'NF_instances':
            #This is a port of the NF. I have to extract the port ID and the type of the NF.
            #XXX I'm using the port ID as name of the port
            vnf = port.get_parent().get_parent()
            vnfType = vnf.type.get_value()
            vnfName = vnf.name.get_value()
            portID = port.id.get_value()
            portName = port.name.get_value()
            dst_node = ('internal',  portName, vnfType, vnfName)
        else:
            logging.error("Invalid port '%s' defined in a flowentry",port)
            error = True
            return

        if src_node[0] == 'internal' and dst_node[0] == 'external' and 'ovs' in src_node[2]:
            vnfName = src_node[3]
            portName = src_node[1]
            port_types.setdefault(vnfName, {})
            port_types[vnfName][portName] = DPPort.External
        elif src_node[0] == 'external' and dst_node[0] == 'internal' and 'ovs' in dst_node[2]:
            vnfName = dst_node[3]
            portName = dst_node[1]
            port_types.setdefault(vnfName, {})
            port_types[vnfName][portName] = DPPort.External
        elif src_node[0] == 'internal' and dst_node[0] == 'internal' \
            and 'ovs' in src_node[2] and 'ovs' in dst_node[2]:
            vnfName = dst_node[3]
            portName = dst_node[1]
            port_types.setdefault(vnfName, {})
            port_types[vnfName][portName] = DPPort.Internal

    return port_types


if __name__ == "__main__":
    xml = open('er_nffg.xml').read()
    DP_switches = process_nffg(xml)
    for DP_name in DP_switches:
        id = DP_switches[DP_name]['id']
        ports = DP_switches[DP_name]['ports']
        new_DP = DP(DP_name, id, ports)
        for port in new_DP.ports:
            print port.ifname
            print port.link_type



