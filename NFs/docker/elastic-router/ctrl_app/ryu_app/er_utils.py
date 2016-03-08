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
        # contains tuple [(match_dict{}, actions[], priority),]
        self.oftable = []

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