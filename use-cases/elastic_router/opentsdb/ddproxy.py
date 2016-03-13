#!/usr/bin/python3
# coding=utf-8
__author__ = 'etamlva'
__author__ = 'berpec'
import json
import argparse
import requests
import logging
from doubledecker.clientSafe import ClientSafe


class DDProxy(ClientSafe):
    def __init__(self, name, dealerurl, customer, keyfile):
        # init DD client
        super().__init__(name, dealerurl, customer, keyfile)

    def on_reg(self):
        self.subscribe('measurements', 'all')

    def on_data(self, msg):
        try:
            message = json.loads(msg[1].decode('utf-8'))
            if "measurement" in message['type']:
                self.push_opentsdb(message['data'])
        except (TypeError, ValueError):
            logging.error(msg.pop(0), 'sent', msg)

    # Override DD client's receiving function to implement alert
    # displaying.
    def on_pub(self, topic, data):
        if topic == "measurements".encode():
            try:
                message = json.loads(data[0].decode('utf-8'))
                logging.info("X", json.dumps(message['data']))
                self.push_opentsdb(message['data'])
            except (TypeError, ValueError):
                logging.error("JSON type or value error during push")
        else:
            logging.warning("Subscribe recive:", data, "on", topic)

    def push_opentsdb(self, data):
        logging.info("push_to_opentsdb:", json.dumps(data))
        try:
            if "os.cpu" in data['id']:
                new_metric = {}
                new_metric['metric'] = data['id']
                new_metric['timestamp'] = data['time']
                new_metric['value'] = data['value']
                new_metric['tags'] = data['tags']
                requests.post('http://127.0.0.1:4242/api/put',
                              data=json.dumps(new_metric))

            if "os.mem" in data['id']:
                new_metric = {}
                new_metric['metric'] = data['id']
                new_metric['timestamp'] = data['time']
                new_metric['value'] = data['value']
                new_metric['tags'] = data['tags']
                requests.post('http://127.0.0.1:4242/api/put',
                              data=json.dumps(new_metric))

            if "overload" in data['id']:
                new_metric = {}
                new_metric['metric'] = data['id']+".tx"
                new_metric['timestamp'] = data['time']
                new_metric['value'] = data['value'][0]
                new_metric['tags'] = data['tags']
                requests.post('http://127.0.0.1:4242/api/put',
                              data=json.dumps(new_metric))

                new_metric = {}
                new_metric['metric'] = data['id']+".rx"
                new_metric['timestamp'] = data['time']
                new_metric['value'] = data['value'][1]
                new_metric['tags'] = data['tags']
                requests.post('http://127.0.0.1:4242/api/put',
                              data=json.dumps(new_metric))

                new_metric = {}
                new_metric['metric'] = data['id']+".rxr"
                new_metric['timestamp'] = data['time']
                new_metric['value'] = data['value'][2]
                new_metric['tags'] = data['tags']
                requests.post('http://127.0.0.1:4242/api/put',
                              data=json.dumps(new_metric))

                new_metric = {}
                new_metric['metric'] = data['id']+".txr"
                new_metric['timestamp'] = data['time']
                new_metric['value'] = data['value'][3]
                new_metric['tags'] = data['tags']
                requests.post('http://127.0.0.1:4242/api/put',
                              data=json.dumps(new_metric))

        except requests.exceptions.ConnectionError:
            logging.warning('Failed to push_opentsdb :(((')

    def on_discon(self):
        logging.info("The client got disconnected")

    def on_error(self, code, msg):
        logging.info("ERROR n#%d : %s", code, msg)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='DoubleDecker OpenTSDB proxy')
    parser.add_argument("name", help="Identity of this client")
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

    logging.info("Safe client")
    dd_proxy = DDProxy(name=args.name.encode('utf8'),
                       dealerurl=args.dealer,
                       customer=args.customer,
                       keyfile=args.keyfile)

    logging.info("Starting DoubleDecker example client")
    logging.info("See ddclient.py for how to send/recive and publish/subscribe")
    dd_proxy.start()
