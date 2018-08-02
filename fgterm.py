import zmq

hostname = "tcp://10.105.22.2:5555"

context = zmq.Context()
print("Connecting")
socket = context.socket(zmq.REQ)
socket.RCVTIMEO = 1000
socket.connect(hostname)
socket.setsockopt(zmq.LINGER, 1000)
print("Connected to {}".format(hostname))

print("Ready for input")
done = False
while not done:
    print("> ", end="")
    i = input()
    socket.send(i.encode('utf-8'))

    try:
        msg = socket.recv()
        print(msg.decode('utf-8'))
    except zmq.error.Again:
        print("Timeout")
        done = True

socket.close()
print("Disconnected from {}".format(hostname))