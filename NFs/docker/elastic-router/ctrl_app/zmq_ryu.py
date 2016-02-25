from eventlet.green import zmq
import eventlet

# https://github.com/eventlet/eventlet/tree/master/examples

CTX = zmq.Context(1)


def alarm_receiver():
    print("STARTING BOB, waiting for ALARM")
    bob = zmq.Socket(CTX, zmq.PULL)
    bob.bind("ipc:///tmp/alarm_trigger")

    while True:
        msg = bob.recv_multipart()
        print("received :", msg)

if __name__ == "__main__":
    alice = zmq.Socket(CTX, zmq.REQ)
    alice.connect("ipc:///tmp/alarm_subscribe")
    alice.send_multipart([b'sub', b'alarms', b'all'])

    sub_server = eventlet.spawn(alarm_receiver)

    sub_server.wait()
