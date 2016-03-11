import re
import urllib2
import copy

from ryu.ofproto import ofproto_v1_3, ofproto_v1_3_parser
from ryu.lib.packet import ether_types, ethernet, ipv4, tcp, arp, udp, icmp, vlan, ipv6, lldp
import logging
#Set the logger
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

class DP:
    """
    class to hold the Data Path instances (ovs switches) of the elastic router
    """
    def __init__(self, nffg_name, nffg_id, ports):
        # ovs bridge name, name from nffg
        self.name = nffg_name
        # ovs id from nffg
        self.id = nffg_id
        # ovs bridge datapath id in ryu
        self.datapath_id = None
        # ovs bridge datapath object in ryu
        self.datapath = None
        # ovs bridge is detected by control app via openflow
        self.registered = False
        # ovs bridge ports
        self.ports = []
        for port in ports:
            if isinstance(port, basestring):
                port_id = len(self.ports)+1
                new_port = DPPort(port, port_id)
                new_port.DP = self
                self.ports.append(new_port)
            elif isinstance(port, DPPort):
                port.DP = self
                self.ports.append(port)

        self.mac_to_port = {}

        # statistics of this DP
        self.port_txstats = {}
        self.port_rxstats = {}
        self.port_txrate = {}
        self.port_rxrate = {}
        self.previous_monitor_time = {} #store time of monitor data to calculate correct timedelta and rx_rate

        # openflow table of this DP
        # contains tuple [(match_dict{}, actions[], priority),
        self.oftable = []
        self.set_default_oftable_new()

        # oftable to translate upon scaling out
        self.scale_out_port_dict = {}

    def set_default_oftable_new(self):
        # send new ARP packets to controller
        priority = 2
        match_dict = create_dictionary(eth_type=ether_types.ETH_TYPE_ARP)
        ofproto = ofproto_v1_3
        parser = ofproto_v1_3_parser
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.oftable.append((match_dict, actions, priority))

        # send new ipv4 packets to controller
        priority = 2
        match_dict = create_dictionary(eth_type=ether_types.ETH_TYPE_IP)
        ofproto = ofproto_v1_3
        parser = ofproto_v1_3_parser
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.oftable.append((match_dict, actions, priority))

        # drop all other packets
        priority = 1
        match_dict = create_dictionary()
        actions = []
        self.oftable.append((match_dict, actions, priority))

    # call this function when new DP is registered and port numbers are known
    def set_default_oftable_scale_out(self):
        # add default flow entries for new DP
        external_ports = [port for port in self.ports if port.port_type == DPPort.External]
        internal_ports = [port for port in self.ports if port.port_type == DPPort.Internal]
        # assume only 1 external port possible
        if len(external_ports) == 1:
            out_port_number = external_ports[0].number
            for int_port in internal_ports:
                priority = 20
                match_dict = create_dictionary(in_port=int_port.number)
                parser = ofproto_v1_3_parser
                actions = [parser.OFPActionOutput(out_port_number)]
                self.oftable.append((match_dict, actions, priority))
         # for multiple external ports, use VLAN IDs


    # call this function when new DP is registered and port numbers are known
    def translate_oftable_scale_out(self):
        scale_out_port_dict = self.scale_out_port_dict
        # add new flow_entries
        for old_in_port in scale_out_port_dict:
            old_DP = old_in_port.DP
            ofproto = old_DP.datapath.ofproto
            new_in_port = scale_out_port_dict[old_in_port]
            new_DP = new_in_port.DP

            # new_DP should be the same as self
            if new_DP.name is not self.name:
                logging.info('This DP:{0} checked DP: {1}, try next...'.format(self.name, new_DP.name))
                continue

            for match_dict, actions, priority  in old_DP.oftable:

                # check if this oftable entry contains a port to be translated
                # (the default port entries have other match entries eg. eth_type)
                if 'in_port' not in match_dict:
                    continue

                logging.info('old DP match dict: {0}'.format(match_dict))
                #logging.info('old_in_port number: {0}'.format(old_in_port.number))
                logging.info('old_in_port : {0}'.format(old_in_port.ifname))
                logging.info('new_in_port : {0}'.format(new_in_port.ifname))

                if match_dict['in_port'] == old_in_port.number:
                    # set new in_port
                    new_match_dict = copy.deepcopy(match_dict)
                    new_match_dict['in_port'] = new_in_port.number

                    # set new out_port
                    new_actions = list(actions)
                    for i in range(0, len(actions)):
                        if actions[i].port == ofproto.OFPP_FLOOD:
                            new_actions[i].port = ofproto.OFPP_FLOOD
                        else:
                            old_outportno = actions[i].port
                            old_outport = old_DP.get_port_by_number(old_outportno)
                            logging.info('old out port: {0}'.format(old_outport.ifname))

                            #for port in scale_out_port_dict:
                            #    logging.info('scale_out_port_dict: {0}'.format(port.ifname))

                            new_outport = scale_out_port_dict[old_outport]
                            dest_DP = new_outport.DP

                            if dest_DP.name == self.name:
                                # in and out port are on the same DP
                                new_outportno = new_outport.number
                                new_actions[i].port = new_outportno
                                break
                            for port in dest_DP.ports:
                                if port.port_type == DPPort.External: continue
                                # out port is located on another DP as in port, check the internal ports
                                if port.linked_port.DP.name == self.name:
                                    #logging.info('new out port remote: {0}'.format(port.ifname))
                                    local_outport = port.linked_port
                                    logging.info('new out port local: {0}'.format(local_outport.ifname))
                                    new_outportno = local_outport.number
                                    logging.info('new local out port number: {0}'.format(local_outport.number))
                                    new_actions[i].port = int(new_outportno)
                                    break
                    self.oftable.append((new_match_dict, new_actions, priority))
        # reset dictionary
        self.scale_out_port_dict = {}


    def get_port(self, port_name=None):
        for port in self.ports:
            if port.ifname == port_name:
                return port

    def get_port_by_number(self, number):
        for port in self.ports:
            if port.number == number:
                return port

    def add_port(self,  ifname, port_type=None, linked_port=None ):
        port_id = len(self.ports)+1
        new_port = DPPort(ifname, port_id, DP_parent=self, port_type=port_type, linked_port=linked_port)
        self.ports.append(new_port)
        return new_port

    def check_connected(self, linked_DP):
        """
        return true is linked_DP is connected with a port to this DP
        """
        for port in self.ports:
            try:
                if port.linked_port.DP is linked_DP:
                    return True
            except:
                continue
        return False


class DPPort:
    """
    class to hold the ports of the DP instances in the elastic router
    """

    Internal = 1
    External = 2
    SAP = 3

    def __init__(self, ifname, id, port_number=None, DP_parent=None, port_type=None, linked_port=None):
        self.ifname = ifname
        self.id = id #port id in nffg
        self.number = port_number
        self.port_type = port_type #internal or external or SAP link
        self.DP = DP_parent
        self.linked_port = linked_port
        #self.virtualizer_port = virtualizer_port # virtualizer port object

    @staticmethod
    def get_datapath_name(ifname):
        # find datapath name starting with 'ER'
        match = re.search('\Aovs\d*(?=_eth\d*)', ifname)
        if match:
            return match.group()
        else:
            return None

def create_dictionary(**kwargs):
    return kwargs