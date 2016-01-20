__author__ = 'Steven Van Rossem'

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3, ofproto_v1_0
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet, ipv4, tcp, arp, udp, icmp, vlan, ipv6, lldp
from ryu.ofproto import ether, inet
from ryu.lib import hub
from ryu.topology import switches
from ryu.topology.event import EventSwitchEnter

from operator import attrgetter
import logging
#Set the logger
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

import urllib2

from virtualizer_er import *
from er_utils import *
from er_monitor import *

class ElasticRouter(app_manager.RyuApp):

    _CONTEXTS = {
        'switches': switches.Switches,
    }

    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]
    # Cf_Or interface available in docker container
    REST_Cf_Or =  'http://172.17.0.1:8080'

    def __init__(self, *args, **kwargs):
        super(ElasticRouter, self).__init__(*args, **kwargs)

        # ovs switches attached to elastic router
        self.DP_instances = {}
        # ovs dpid to DP object
        self.DPIDtoDP = {}

        # get deployed nffg and check which ovs switches are in it
        #xml = open('get-config.nffg')
        xml = self.get_nffg()
        DP_switches = process_nffg(xml)

        for DP_name in DP_switches:
            id = DP_switches[DP_name]['id']
            ports = DP_switches[DP_name]['ports']
            new_DP = DP(DP_name, id, ports)
            self.DP_instances[DP_name] = new_DP

        self.logger.debug('DP instances: {0}'.format(self.DP_instances))

        self.monitorApp = ElasticRouterMonitor(self)
        # monitor function to trigger nffg change
        self.monitor_thread = hub.spawn(self._monitor)

    # monitor stats and trigger scaling
    def _monitor(self):
        while True:
            # check if all switches are detected
            registered_DPs = filter(lambda x: self.DP_instances[x].registered is True, self.DP_instances)
            self.logger.info('{0} switches detected'.format(len(registered_DPs)))

            # ask port statistics
            self.monitorApp.init_measurement()
            # check if measurements are valid
            if not self.monitorApp.check_measurement():
                hub.sleep(1)
                continue
            # do measurements (cpu, ingress rate)
            self.monitorApp.do_measurements()

            # print some statistics
            for DP_name in self.DP_instances:
                DP = self.DP_instances.get(DP_name)
                self.logger.info("{0} total ingress rate: {1} pps".format(DP_name, self.monitorApp.DP_ingress_rate[DP_name]))
                self.logger.info("total ingress rate: {0} pps".format(self.monitorApp.complete_ingress_rate))
            # check if scaling is needed
            if len(self.DP_instances) > 1:
                # only scale in multiple DPs to 1 datapath
                self.monitorApp.check_scaling_in()
            else:
                # only scale out if 1 DP is in the nffg
                self.monitorApp.check_scaling_out()

            hub.sleep(1)


    # new switch detected
    @set_ev_cls(EventSwitchEnter)
    def _ev_switch_enter_handler(self, ev):
        datapath = ev.switch.dp
        self.logger.info('registered OF switch id: %s' % datapath.id)
        ofproto = datapath.ofproto
        self.logger.info('OF version: {0}'.format(ofproto))
        #print 'switch entered send port desc request'
        self.send_port_desc_stats_request(datapath)

    # query ports of new detected switch
    def send_port_desc_stats_request(self, datapath):
        ofp_parser = datapath.ofproto_parser
        req = ofp_parser.OFPPortDescStatsRequest(datapath, 0)
        datapath.send_msg(req)

    # register this switch and get assigned port numbers
    @set_ev_cls(ofp_event.EventOFPPortDescStatsReply, MAIN_DISPATCHER)
    def port_desc_stats_reply_handler(self, ev):
        ports = []
        this_DP = None
        # TODO check ovs instances by datapath id instead of name deduction from port name
        for p in ev.msg.body:
            # skip LOCAL port
            if p.port_no > 100 :
                continue
            ovs_name = DPPort.get_datapath_name(p.name)
            self.logger.info('found OVS name:{0} port: {1}'.format(ovs_name, p.name))
            this_DP = self.DP_instances.get(ovs_name)
            if not this_DP:
                continue

            # set port number assigned by orchestrator to this interface
            port = this_DP.get_port(p.name)
            port.number = p.port_no

            ports.append('port_no=%d hw_addr=%s name=%s config=0x%08x '
                         'state=0x%08x curr=0x%08x advertised=0x%08x '
                         'supported=0x%08x peer=0x%08x curr_speed=%d '
                         'max_speed=%d' %
                         (p.port_no, p.hw_addr,
                          p.name, p.config,
                          p.state, p.curr, p.advertised,
                          p.supported, p.peer, p.curr_speed,
                          p.max_speed))
        self.logger.debug('OFPPortDescStatsReply received: %s', ports)

        if not this_DP:
            return

        this_DP.datapath = ev.msg.datapath
        this_DP.datapath_id = ev.msg.datapath.id
        this_DP.registered = True
        self.DPIDtoDP[this_DP.datapath_id] = this_DP
        self.logger.info('stored OF switch id: %s' % this_DP.datapath_id)

    # query port statistics
    def port_stats_request(self, datapath):
        #print 'request stats'
        #self.logger.info('send stats request: %016x', datapath.id)
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        req = parser.OFPPortStatsRequest(datapath, 0, ofproto.OFPP_ANY)
        datapath.send_msg(req)

    @set_ev_cls(ofp_event.EventOFPPortStatsReply, MAIN_DISPATCHER)
    def _port_stats_reply_handler(self, ev):
        #print 'receive stats'
        body = ev.msg.body

        dpid = ev.msg.datapath.id
        this_DP = self.DPIDtoDP[dpid]


        for stat in sorted(body, key=attrgetter('port_no')):
            #skip control port
            if stat.port_no > 20 :
                continue
            #self.logger.debug('port of this DP: {0} port: {1}'.format(this_DP.name, stat.port_no))
            '''
            self.logger.info('%016x %8x %8d %8d %8d %8d %8d %8d',
                             ev.msg.datapath.id, stat.port_no,
                             stat.rx_packets, stat.rx_bytes, stat.rx_dropped,
                             stat.tx_packets, stat.tx_bytes, stat.tx_dropped)
            '''

            #calculate time delta since last measurement
            port_uptime  = stat.duration_sec + stat.duration_nsec * 10**(-9)
            this_DP.previous_monitor_time.setdefault(stat.port_no,0)
            previous_time = this_DP.previous_monitor_time[stat.port_no]
            if previous_time <= 0 or previous_time > port_uptime:
               # first measurement
               #print 'ER: {0}-{1}'.format(ER.name, stat.port_no)
               #print 'first measurement'
               #print 'previous time: {0}'.format(previous_time)
               #print 'port_uptime: {0}'.format(port_uptime)
               this_DP.port_txstats[stat.port_no] = stat.tx_packets
               this_DP.port_rxstats[stat.port_no] = stat.rx_packets
               this_DP.previous_monitor_time[stat.port_no] = port_uptime
               return
            else :
               time_delta = (port_uptime - previous_time)
               #print 'ER: {0}-{1}'.format(ER.name, stat.port_no)
               #self.logger.debug('time delta: {0}'.format(time_delta))
               #print 'previous time: {0}'.format(previous_time)
               #print 'port_uptime: {0}'.format(port_uptime)


            if this_DP and stat.port_no in this_DP.port_txstats :
               this_DP.port_txrate[stat.port_no] = (stat.tx_packets - this_DP.port_txstats[stat.port_no])/float(time_delta)
               #print(this_DP.port_txrate[stat.port_no])
            if this_DP and stat.port_no in this_DP.port_rxstats :
               this_DP.port_rxrate[stat.port_no] = (stat.rx_packets - this_DP.port_rxstats[stat.port_no])/float(time_delta)
               #self.logger.debug('rx rate: {0}'.format(this_DP.port_rxrate[stat.port_no]))

            this_DP.port_txstats[stat.port_no] = stat.tx_packets
            this_DP.port_rxstats[stat.port_no] = stat.rx_packets
            this_DP.previous_monitor_time[stat.port_no] = port_uptime


    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        # If you hit this you might want to increase
        # the "miss_send_length" of your switch
        if ev.msg.msg_len < ev.msg.total_len:
            self.logger.debug("packet truncated: only %s of %s bytes",
                              ev.msg.msg_len, ev.msg.total_len)
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']
        priority = 1

        pkt = packet.Packet(msg.data)
        '''
        for p in pkt.protocols:
            print p
        print ip_proto
        print pkt.get_protocol(ip_proto)
        '''
        eth = pkt.get_protocols(ethernet.ethernet)[0]
        mac_dst = eth.dst
        mac_src = eth.src
        dpid = datapath.id
        #self.logger.info("packet in %s %s %s %s %s %s", dpid, mac_src, mac_dst, in_port, eth.ethertype)

        dpid = datapath.id
        if dpid not in self.DPIDtoDP:
            self.logger.info( 'exception in packet_in handler')
            return
        source_DP = self.DPIDtoDP[dpid]
        # monitor packet_in rate per port
        #source_DP.packet_in_port_count.setdefault(in_port, 0)
        #source_DP.packet_in_port_count[in_port] += 1
        #self.logger.debug("packet in count port:{0} = {1}".format(in_port, source_DP.packet_in_port_count[in_port]))

        #source_ER.mac_to_port.setdefault(dpid, {})

        #learn a mac address to avoid FLOOD next time.
        source_DP.mac_to_port[mac_src] = in_port
        #no FLOOD -> loops in topology!

        if mac_dst in source_DP.mac_to_port:
            out_port = source_DP.mac_to_port[mac_dst]
        else:
            out_port = ofproto.OFPP_FLOOD

        actions = [parser.OFPActionOutput(out_port)]
        # install a flow to avoid packet_in next time
        if out_port != ofproto.OFPP_FLOOD:
            match = parser.OFPMatch(in_port=in_port, eth_dst=mac_dst)
            # verify if we have a valid buffer_id, if yes avoid to send both
            # flow_mod & packet_out
            if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                self.add_flow(datapath, priority, match, actions, msg.buffer_id)
                return
            else:
                self.add_flow(datapath, priority, match, actions)
        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        datapath.send_msg(out)

        return



        pkt_ethernet = pkt.get_protocol(ethernet.ethernet)
        if not pkt_ethernet:
            # which packet is this?
            self.logger.info('This is no ethernet packet')
            return
        pkt_arp = pkt.get_protocol(arp.arp)
        if pkt_arp:
            self.logger.info('received ARP packet')
            self._handle_arp(source_DP, in_port, pkt_ethernet, pkt_arp, msg)
            return

        pkt_ipv4 = pkt.get_protocol(ipv4.ipv4)
        if pkt_ipv4:
            self.logger.info('received ipv4 packet')
            dst = pkt_ipv4.dst
            '''
            dst_SAP = self.IP_to_SAP[dst]
            dst_port = source_DP.SAP_to_port[dst_SAP].number
            actions = [parser.OFPActionOutput(dst_port)]
            #actions[0].port = 10
            '''
            ip_proto = pkt_ipv4.proto
            #match = self.getFullMatch(msg)

            pkt_tcp = pkt.get_protocol(tcp.tcp)
            pkt_udp = pkt.get_protocol(udp.udp)
            pkt_icmp = pkt.get_protocol(icmp.icmp)
            if pkt_tcp:
                self.logger.info('received tcp packet')
                L4_pkt = pkt_tcp
                L4_src = L4_pkt.src_port
                L4_dst = L4_pkt.dst_port
                match_dict = create_dictionary(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                                    ip_proto=ip_proto, tcp_src=L4_src, tcp_dst=L4_dst)
                #match = parser.OFPMatch(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                #                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                #                    ip_proto=ip_proto, tcp_src=L4_src, tcp_dst=L4_dst)
                priority = 10
            elif pkt_udp:
                self.logger.info('received udp packet')
                L4_pkt = pkt_udp
                L4_src = L4_pkt.src_port
                L4_dst = L4_pkt.dst_port
                match_dict = create_dictionary(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                                    ip_proto=ip_proto, udp_src=L4_src, udp_dst=L4_dst)

                #match = parser.OFPMatch(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                #                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                #                    ip_proto=ip_proto, udp_src=L4_src, udp_dst=L4_dst)


                #match = getFullMatch(msg)
                priority = 10
            elif pkt_icmp:
                self.logger.info('received icmp packet')
                match_dict = create_dictionary(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                                    ip_proto=ip_proto)
                #match = parser.OFPMatch(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                #                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                #                    ip_proto=ip_proto)
            else:
                self.logger.info('received other packet')
                match_dict = create_dictionary(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                                    ip_proto=ip_proto)
                #match = parser.OFPMatch(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                #                    eth_type=ether.ETH_TYPE_IP,ipv4_dst=pkt_ipv4.dst, ipv4_src=pkt_ipv4.src,
                #                    ip_proto=ip_proto)
                #match = getFullMatch(msg)


            #add flow
            #source_DP.oftable.append((match_dict, actions, priority))
            match = parser.OFPMatch(**match_dict)
            if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                self.add_flow(datapath, priority, match, actions, msg.buffer_id)
                return
            else:
                self.add_flow(datapath, priority, match, actions)

            #send packet out
            data = None
            if msg.buffer_id == ofproto.OFP_NO_BUFFER:
                data = msg.data
            out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                      in_port=in_port, actions=actions, data=data)
            datapath.send_msg(out)
            return

        pkt_lldp = pkt.get_protocol(lldp.lldp)
        if pkt_lldp:
            self.logger.info('received lldp packet (unhandled)')
            match_dict = create_dictionary(in_port=in_port, eth_dst=mac_dst, eth_src=mac_src,
                                           eth_type=ether.ETH_TYPE_LLDP)
            return

        pkt_ipv6 = pkt.get_protocol(ipv6.ipv6)
        if pkt_ipv6:
            return
            self.logger.info('unhandled ipv6 packet_in message on {0} port {1}'.format(self.DPIDtoER[datapath.id].name, in_port))
            for p in pkt.protocols:
                self.logger.info('protocol:{0}'.format(p))
        else:
            self.logger.info('unhandled packet_in message')
            self.logger.info("packet in %s %s %s %s %s", dpid, mac_src, mac_dst, in_port, eth.ethertype)
            for p in pkt.protocols:
                self.logger.info('protocol:{0}'.format(p))

    def _handle_arp(self, source_DP, in_port, pkt_ethernet, pkt_arp, msg):
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        dst = pkt_ethernet.dst
        out_port = []
        if dst in source_DP.mac_to_port:
            out_port.append(source_DP.mac_to_port[dst])
        else:
            # TODO no FLOOD -> loops in topology!
            #input in internal port = output only via external port and vice versa
            out_port.append(ofproto.OFPP_FLOOD)
            #print source_ER.portno_to_type
            '''
            if source_DP.portno_to_type[in_port] == ERPort.External:
                out_port.append(ofproto.OFPP_FLOOD)
            elif source_DP.portno_to_type[in_port] == ERPort.Internal:
                for port in source_DP.portno_to_type:
                    if source_DP.portno_to_type[port] == ERPort.External:
                        out_port.append(port)
            '''
        actions = []
        for port in out_port:
            actions.append(parser.OFPActionOutput(port))

        # install a flow to avoid packet_in next time
        priority = 1
        # TODO only install flow entry for ARP?
        #match_dict = create_dictionary(eth_type=ether.ETH_TYPE_ARP,in_port=in_port, eth_dst=dst)
        match_dict = create_dictionary(in_port=in_port, eth_dst=dst)
        #source_DP.oftable.append((match_dict, actions, priority))
        match = parser.OFPMatch(**match_dict)
        #match = parser.OFPMatch(eth_type=ether.ETH_TYPE_ARP,in_port=in_port, eth_dst=dst)
        #match = self.getFullMatch(msg)
        # verify if we have a valid buffer_id, if yes avoid to send both
        # flow_mod & packet_out
        if msg.buffer_id != ofproto.OFP_NO_BUFFER:
            self.add_flow(datapath, priority, match, actions, msg.buffer_id)
            return
        else:
            self.add_flow(datapath, priority, match, actions)

        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        datapath.send_msg(out)

    def _send_packet(self, datapath, port, pkt):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        pkt.serialize()
        #self.logger.info("packet-out %s" % (pkt,))
        data = pkt.data
        actions = [parser.OFPActionOutput(port=port)]
        out = parser.OFPPacketOut(datapath=datapath,
                                  buffer_id=ofproto.OFP_NO_BUFFER,
                                  in_port=ofproto.OFPP_CONTROLLER,
                                  actions=actions,
                                  data=data)
        datapath.send_msg(out)

    def get_nffg(self):
        url = self.REST_Cf_Or + '/get-config'
        print url
        # by default a GET request is created:
        req = urllib2.Request(url, '')
        nffg_xml = urllib2.urlopen(req).read()
        return nffg_xml

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,actions)]
        if buffer_id:
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,
                                    priority=priority, match=match,
                                    instructions=inst)
            print 'add flow\n'
        else:
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                    match=match, instructions=inst)
        datapath.send_msg(mod)