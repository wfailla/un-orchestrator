__author__ = 'Administrator'

from eventlet.green import zmq
import eventlet
from ryu.lib import hub
import logging
#Set the logger
#logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)
logging.basicConfig(level=logging.DEBUG)

CTX = zmq.Context(1)

class er_zmq:
    def __init__(self):
        #Set the logger
        #logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)
        self.log = logging.getLogger('zmq_logger')
        self.log.setLevel(logging.DEBUG)

        #self.CTX = zmq.Context(1)
        logging.debug("STARTING ZMQ")
        #self.zmq_pull_thread = hub.spawn(self.bob_client(CTX))

        self.alarm_subscribe()

        sub_server = hub.spawn(self.alarm_receiver)
        #sub_server.wait()

    def bob_client(self):
        logging.info("STARTING BOB")
        bob = zmq.Socket(CTX, zmq.PULL)
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

    def alarm_subscribe(self):
        self.log.debug("ALARM SUBSCRIBE")
        alice = zmq.Socket(CTX, zmq.REQ)
        alice.connect("ipc:///tmp/alarm_subscribe")
        alice.send_multipart([b'sub', b'alarms', b'all'])

    def alarm_receiver(self):
        self.log.debug("STARTING BOB, waiting for ALARM")
        bob = zmq.Socket(CTX, zmq.PULL)
        bob.bind("ipc:///tmp/alarm_trigger")

        while True:
            msg = bob.recv_multipart()

            print("received :", msg)
            self.log.debug("STARTING BOB, waiting for ALARM3")
