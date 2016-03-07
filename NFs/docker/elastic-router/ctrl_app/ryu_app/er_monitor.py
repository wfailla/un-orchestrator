__author__ = 'Steven Van Rossem'

import logging
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

from er_utils import *


class ElasticRouterMonitor:

    def __init__(self, ERctrlapp):

        self.ERctrlapp = ERctrlapp

        # statistics of last measurement
        self.complete_ingress_rate = 0
        self.DP_ingress_rate = {}

        # thresholds to trigger scaling action
        self.ingress_rate_upper_threshold = 70
        self.ingress_rate_lower_threshold = 30


    def init_measurement(self):
        self.DP_ingress_rate = {}
        self.complete_ingress_rate = 0

        # query port statistics for each DP
        registered_DPs = filter(lambda x: self.ERctrlapp.DP_instances[x].registered is True, self.ERctrlapp.DP_instances)
        for DP_name in registered_DPs:
            self.DP_ingress_rate.setdefault(DP_name,0)

            DP = self.ERctrlapp.DP_instances.get(DP_name)
            self.ERctrlapp.port_stats_request(DP.datapath)

    # True if measurement is valid and ready for analysis
    def check_measurement(self):
        registered_DPs = filter(lambda x: self.ERctrlapp.DP_instances[x].registered is True, self.ERctrlapp.DP_instances)
        # all datapaths are registered?
        if len(registered_DPs) != len(self.ERctrlapp.DP_instances):
            return False
        # enough measurements collected?
        for DP_name in self.ERctrlapp.DP_instances:
            if len(self.ERctrlapp.DP_instances[DP_name].port_rxrate) < 1:
                return False

        return True

    def do_measurements(self):
        for DP_name in self.ERctrlapp.DP_instances:
            DP = self.ERctrlapp.DP_instances.get(DP_name)
            self.check_ingress_rate(DP)

    def check_ingress_rate(self, DP):
        for n in DP.port_rxrate:
            self.DP_ingress_rate[DP.name] += int(DP.port_rxrate[n])
            if DP.get_port_by_number(n).link_type ==  DPPort.External :
                self.complete_ingress_rate += int(DP.port_rxrate[n])

    def check_scaling_out(self):
        #  only scale out in case of single DP
        logging.info('TODO: send scaled out nffg...')
        if len(self.ERctrlapp.DP_instances) != 1:
            return

        scaling_out_ports = []
        non_scaling_ports = []
        # get first DP
        this_DP = self.ERctrlapp.DP_instances.itervalues().next()
        if self.DP_ingress_rate[this_DP.name] > self.ingress_rate_upper_threshold:
            for port in this_DP.ports:
                scaling_out_ports.append(port.ifname)

        if len(scaling_out_ports) > 0:
            logging.info('scaling out: {0} ports: {1}'.format(this_DP.name,scaling_out_ports ))

    def check_scaling_in(self):
        return