#!/usr/bin/python3
# -*- coding: utf-8 -*-

# Simple code to simulate the input for the aggregator.

import argparse
import logging
from doubledecker.clientSafe import ClientSafe
import json

# Inherit ClientSafe and implement the abstract classes
# ClientSafe does encryption and authentication using ECC (libsodium/nacl)


class TestCli(ClientSafe):
    def __init__(self, name, dealerurl, customer, keyfile):
        super().__init__(name, dealerurl, customer, keyfile)

    # callback called automatically everytime a point to point is sent at
    # destination to the current client
    def on_data(self, src, msg):
        print("DATA from %s: %s" % (str(src), str(msg)))

    # callback called upon registration of the client with its broker
    def on_reg(self):
        print("The client is now connected")

        # this function notifies the broker that the client is interested
        # in the topic 'monitoring' and the scope should be 'all'
        self.subscribe('monitoring', 'all')

        # this function sends a point to point message to a client named
        # 'another client', if that client doesn't exist an error message
        # will be received warning that 'another_client' doesn't exists
#        self.sendmsg('another_client', 'Hello other client')

        # this function publishes a message on the topic 'monitoring'
        # in this code example the client will receive it because it has
        # subscribed to the same topic just before
#        self.publish('monitoring', 'cpu usage:123%')

        rpc_obj = {"jsonrpc": "2.0", "method": "add_monitor", "params": {"name": "m3" }}
        rpc_obj_json = json.dumps(rpc_obj)
        self.publish('monitor_aggregate', rpc_obj_json)
        rpc_obj = {"jsonrpc": "2.0", "method": "add_monitor", "params": {"name": "m4" }}
        rpc_obj_json = json.dumps(rpc_obj)
        self.publish('monitor_aggregate', rpc_obj_json)
#        rpc_obj = {"jsonrpc": "2.0", "method": "rate_data", "params": {"insert": "SELECT datname FROM pg_database WHERE datistemplate = false;"}}
        rpc_obj = {"jsonrpc": "2.0", "method": "rate_data", "params": {"name": "m3", "lm": 0.69, "lsd": 0.69}}
        rpc_obj_json = json.dumps(rpc_obj)
        self.publish('monitor_aggregate', rpc_obj_json)
        rpc_obj = {"jsonrpc": "2.0", "method": "rate_data", "params": {"name": "m4", "lm": 0.69, "lsd": 0.69}}
        rpc_obj_json = json.dumps(rpc_obj)
        self.publish('monitor_aggregate', rpc_obj_json)
        rpc_obj = {"jsonrpc": "2.0", "method": "rate_data", "params": {"name": "m2", "lm": 0.69, "lsd": 0.69}}
        rpc_obj_json = json.dumps(rpc_obj)
        self.publish('monitor_aggregate', rpc_obj_json)
        rpc_obj = {"jsonrpc": "2.0", "method": "rate_data", "params": {"name": "m1", "lm": 0.69, "lsd": 0.69}}
        rpc_obj_json = json.dumps(rpc_obj)
        self.publish('monitor_aggregate', rpc_obj_json)


    # callback called when the client detects that the heartbeating with
    # its broker has failed, it can happen if the broker is terminated/crash
    # or if the link is broken
    def on_discon(self):
        print("The client got disconnected")

        # this function shuts down the client in a clean way
        # in this example it exists as soon as the client is disconnected
        # fron its broker
        self.shutdown()

    # callback called when the client receives an error message
    def on_error(self, code, msg):
        print("ERROR n#%d : %s" % (code, msg))

    # callback called when the client receives a message on a topic he
    # subscribed to previously
    def on_pub(self, src, topic, msg):
        print("PUB %s from %s: %s" % (str(topic), str(src), str(msg)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generic message client")
    parser.add_argument('name', help="Identity of this client")
    parser.add_argument('customer', help="Name of the customer to get the keys (i.e. 'a' for the customer-a.json file)")
    parser.add_argument(
        '-d',
        "--dealer",
        help='URL to connect DEALER socket to, "tcp://1.2.3.4:5555"',
        nargs='?',
        default='tcp://127.0.0.1:5555')
    parser.add_argument(
        '-f',
        "--logfile",
        help='File to write logs to',
        nargs='?',
        default=None)
    parser.add_argument(
        '-l',
        "--loglevel",
        help='Set loglevel (DEBUG, INFO, WARNING, ERROR, CRITICAL)',
        nargs='?',
        default="INFO")
    parser.add_argument(
        '-k',
        "--keyfile",
        help='File containing the encryption/authentication keys)',
        nargs='?',
        default='')

    args = parser.parse_args()

    numeric_level = getattr(logging, args.loglevel.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.loglevel)

    logging.basicConfig(format='%(levelname)s:%(message)s', filename=args.logfile, level=numeric_level)

    logging.info("Safe client")
    genclient = TestCli(name=args.name,
                          dealerurl=args.dealer,
                          customer=args.customer,
                          keyfile=args.keyfile)

    logging.info("Starting DoubleDecker example client")
    logging.info("See ddclient.py for how to send/recive and publish/subscribe")
    genclient.start()
