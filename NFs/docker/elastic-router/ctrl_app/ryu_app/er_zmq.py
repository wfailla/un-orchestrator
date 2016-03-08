__author__ = 'Administrator'

from eventlet.green import zmq
import eventlet
from ryu.lib import hub
import logging
#Set the logger
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)


class er_zmq:
    def __init__(self):
        #Set the logger
        #logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

        self.CTX = zmq.Context(1)
        logging.debug("STARTING ZMQ")
        #self.zmq_pull_thread = hub.spawn(self.bob_client(CTX))


    def bob_client(self, ctx):
        logging.debug("STARTING BOB")
        bob = zmq.Socket(ctx, zmq.PULL)
        bob.bind("ipc:///tmp/alarm_trigger")
        # if a remote server, tcp connection would be needed

        #while True:
        #    logging.debug("BOB PULLING")
        #    logging.debug("BOB GOT:", bob.recv())

        while True:
            logging.debug("BOB PULLING")
            msg = bob.recv_multipart()
            print("received :", msg)
            hub.sleep(1)

    def test(self):
        while True:
            hub.sleep(2)