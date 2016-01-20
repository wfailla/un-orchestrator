import re
import urllib2

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
                new_port = DPPort(port)
                self.ports.append(new_port)
            elif isinstance(port, DPPort):
                self.ports.append(port)

        self.mac_to_port = {}

        # statistics of this DP
        self.port_txstats = {}
        self.port_rxstats = {}
        self.port_txrate = {}
        self.port_rxrate = {}
        self.previous_monitor_time = {} #store time of monitor data to calculate correct timedelta and rx_rate

    def get_port(self, port_name=None):
        for port in self.ports:
            if port.ifname == port_name:
                return port

    def get_port_by_number(self, number):
        for port in self.ports:
            if port.number == number:
                return port


class DPPort:
    """
    class to hold the ports of the DP instances in the elastic router
    """

    Internal = 1
    External = 2

    def __init__(self, ifname, port_number=None):
        self.ifname = ifname
        self.number = port_number
        self.link_type = None #internal or external (SAP) link

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