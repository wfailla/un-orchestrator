#!/usr/bin/python3
# -*- coding: utf-8 -*-
__author__ = 'eponsko'
from doubledecker.client import ClientUnsafe
from doubledecker.client import ClientSafe
import argparse
import time
import json
import logging
import sys


class SchemaRepository(ClientSafe):
    def __init__(self):
        super().__init__()
        self.tools = None
        self.results = None
        self.readDB()

    def config(self, name, dealerURL, customer):
        super().config(name, dealerURL, customer)

    def on_data(self, src, msg):
        src = src.decode()
        msg = msg.decode()
        try:
            jmsg = json.loads(msg)
        except Exception as e:
            logging.error("Could not parse message: %s" % str(e))
            self.replyError(src, "-1", "Misformed JSON, could not parse!")
            return
        self.processJSON(src, jmsg)

    def on_reg(self):
        logging.info('Registered on the bus')

    def on_pub(self, src, topic, msg):
        logging.info('Sub: %s %s %s' % (str(msg), str(topic), str(src)))

    def sendTime(self):
        logging.info("Sending time")
        try:
            self.sendmsg("latency_recv", str(time.time()))
            self.publish("latency", str(time.time()))
        except ConnectionError as e:
            logging.error("Error sending: ", e)

    def processJSON(self, src, jmsg):
        #        print("src: ", type(src))
        #        print("jmsg: ", type(jmsg))
        #        print("jmsg: ", jmsg)
        logging.info("Src:  %s" % str(src))
        logging.info("JSON: %s" % str(jmsg))

        if "xid" not in jmsg:
            self.replyError(src, '-1', "Misformed JSON, 'xid' missing")
            return
        if "command" in jmsg:
            if 'getTool' == jmsg['command']:
                self.processGetTool(src, jmsg['xid'], jmsg['args'])
            elif 'getTools' == jmsg['command']:
                self.processGetTools(src, jmsg['xid'])
            elif 'getResult' == jmsg['command']:
                self.processGetResult(src, jmsg['xid'], jmsg['args'])
            elif 'getResults' == jmsg['command']:
                self.processGetResults(src, jmsg['xid'])
            else:
                logging.warning("got unknown 'command' %s" % jmsg['command'])
                self.replyError(src, jmsg['xid'], "unknown command")
        else:
            logging.warning("got unknown JSON %s" % jmsg)
            self.replyError(src, jmsg['xid'], "Misformed JSON, 'command' missing")

    def replyError(self, src, xid, msg):
        logging.warning('reply_error: %s %s %s' % (src, str(xid), msg))
        self.sendmsg(src, json.dumps({"result": "error", "data": msg, "xid": xid}).encode())

    def processGetTool(self, src, xid, argument):
        msg = []
        try:
            toolname = argument[0]
        except Exception:
            self.replyError(src, xid, "Misformed JSON, 'getTool' needs an argument!")
            return
        if toolname in self.tools:
            msg = self.tools[toolname]
        jmsg = json.dumps({"result": "ok", "data": msg, "xid": xid})
        self.sendmsg(src, jmsg)

    def processGetTools(self, src, xid):
        msg = list(self.tools)
        self.sendmsg(src, json.dumps({"result": "ok", "data": msg, "xid": xid}))

    def processGetResult(self, src, xid, argument):
        msg = []
        try:
            resultname = argument[0]
        except Exception:
            self.replyError(src, xid, "Misformed JSON, 'getResult' needs an argument!")
            return

        print(self.results)
        if resultname in self.results:
            msg = self.results[resultname]

        print("getResult MSG:, ", msg)

        self.sendmsg(src, json.dumps({"result": "ok", "data": msg, "xid": xid}))

    def processGetResults(self, src, xid):
        msg = list(self.results)
        self.sendmsg(src, json.dumps({"result": "ok", "data": msg, "xid": xid}))

    def readDB(self):
        try:
            f = open('repository.json')
            data = json.load(f)
            self.tools = data['tools']
            # self.results = data['results']
            self.results = dict()
            f.close()
        except Exception as e:
            logging.critical("Could not read database: %s" % str(e))
            sys.exit(1)

        for n in self.tools:
            tool = self.tools[n]
            for r in tool['results']:
                if r in self.results:
                    if n not in self.results[r]:
                        self.results[r].append(n)
                else:
                    self.results[r] = []
                    self.results[r].append(n)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Schema repository")
    parser.add_argument('-d', "--dealer", help='URL to connect DEALER socket to, "tcp://1.2.3.4:5555"',
                        nargs='?', default='tcp://127.0.0.1:5555')
    parser.add_argument('-l', "--loglevel", help='Set loglevel (DEBUG, INFO, WARNING, ERROR, CRITICAL)',
                        nargs='?', default="INFO")

    args = parser.parse_args()
    numeric_level = getattr(logging, args.loglevel.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.loglevel)

    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
    logging.info("test test")
    repo = SchemaRepository()
    repo.config(name='measure-schema', dealerURL=args.dealer, customer='public')
    repo.start()
