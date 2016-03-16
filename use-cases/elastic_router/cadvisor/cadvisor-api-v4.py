#!/usr/bin/python3
# -*- coding: utf-8 -*-

__license__ = """
    This file is part of HMD (Hierarchical MajorDomo).

    HMD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    HMD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MDP.  If not, see <http://www.gnu.org/licenses/>.
"""

__author__ = 'Tamas Levai'
__email__ = 'tamas.levai@ericsson.com'

__author__ = 'Bertrand Pechenot'
__email__ = 'berpec@acreo.se'

import sys
sys.path.insert(0, './doubledecker')
import datetime
import argparse
import requests
import json

from doubledecker.clientSafe import ClientSafe
from zmq.eventloop.ioloop import PeriodicCallback


class cAdvClient(ClientSafe):
    '''
    cAdvisor DoubleDecker client
    '''

    def __init__(self, name, dealerurl, customer, keyfile, to_monitor):
        super().__init__(name, dealerurl, customer, keyfile)

        # init cadvisor aggregating related variables
        self.t = [0, 0, 0, 0, 0]
        self.it = 0
        # we run the container with --net=host arg, so the simplest
        # method of accessing cadvisor is connecting via localhost
        self.url = ('http://localhost:8080/api/v2.0/stats/' +
                    to_monitor +
                    '?type=docker')
        print(self.url)
        self.monitor_loop = PeriodicCallback(self.monitor_cadvisor, 1000)

    def on_data(self, msg):
        pass

    def on_pub(self, src, topic, msg):
        pass

    def on_reg(self):
        pass

    def on_discon(self):
        pass

    def on_error(self, code, msg):
        pass

    def __convert_timestamp_to_diff(self, timestamp):
        '''
        Convert cAdvisor timestamps to nanoseconds for the sake of
        timestamp comparation.
        '''
        ts_s = int(datetime.datetime.strptime(
            timestamp.split('.')[0], "%Y-%m-%dT%H:%M:%S").strftime("%s"))
        ts_ns = int(timestamp.split('.')[1][:-1])
        t = [a*b for a, b in zip([1000000000, 1], [ts_s, ts_ns])]
        return t[0] + t[1]

    def monitor_cadvisor(self):
        '''
        Get cAdvisor statistics via its RESTful API, then publish cpu[%] and
        memory[megabyte] usage on the message bus.
        '''
        try:
            r = requests.get(self.url)
            if r.status_code != 200:
                print("error:", r.status_code)
                return None

            j = r.json()
            for i in j:
                timestamp = datetime.datetime.strptime(
                    j.get(i)[0]['timestamp'].split('.')[0],
                    "%Y-%m-%dT%H:%M:%S").strftime("%s")

                # memory used by the container in megabytes
                mem_usage = int(j.get(i)[0]['memory']['usage']) / 1000000

                cpu_usage_now = j.get(i)[0]['cpu']['usage']['total']
                cpu_usage_prev = j.get(i)[-1]['cpu']['usage']['total']
                timestamp_now = self.__convert_timestamp_to_diff(
                    j.get(i)[0]['timestamp'])
                timestamp_prev = self.__convert_timestamp_to_diff(
                    j.get(i)[-1]['timestamp'])
                # cpu usage percentage
                cpu_load = (float(cpu_usage_now - cpu_usage_prev) /
                            float(timestamp_now - timestamp_prev)) * 100
                if cpu_load > 100:
                    cpu_load = 100.0
                if cpu_load < 0:
                    cpu_load = 0.0

                mem_message = {"type": 'measurement',
                               "data": {"id": "os.mem",
                                        "time": timestamp,
                                        "tags": {"host": self.name.decode()},
                                        "value": mem_usage}}
                cpu_message = {"type": 'measurement',
                               "data": {"id": "os.cpu",
                                        "time": timestamp,
                                        "tags": {"host": self.name.decode()},
                                        "value": cpu_load}}

                self.publish("measurements".encode(),
                             json.dumps(mem_message).encode())
                self.publish("measurements".encode(),
                             json.dumps(cpu_message).encode())

                print(json.dumps(mem_message))
                print(json.dumps(cpu_message))
                print("\n")

        except requests.exceptions.RequestException:
            print("RequestException when connecting to cAdvisor")
            return None


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="CAdvisor Aggregator DoubleDecker Client")
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
    parser.add_argument('-c',
                        '--container',
                        help='Name of the container to monitor')

    args = parser.parse_args()

    client = cAdvClient(
        name=args.name,
        dealerurl=args.dealer,
        customer=args.customer,
        keyfile=args.keyfile,
        to_monitor=args.container)
    try:
        client.start()
    except KeyboardInterrupt as e:
        print("KeyboardInterrupt: ", e)
        client.shutdown()
        print("Done!")
