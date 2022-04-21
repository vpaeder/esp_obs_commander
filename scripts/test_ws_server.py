#!python3
'''This script creates a WebSocket server listening on given host:port.
Run the script without arguments to see detailed usage instructions.
'''
from websocket_server import WebsocketServer
import sys, getopt
import json
import random
import hashlib
import base64

err = False # set to True if and error occurs

try:
    opts, args = getopt.getopt(sys.argv[1:],"h:p:",["host=","port=","password="])
except getopt.GetoptError:
    err = True

# default values
host = None
port = 4455
password = ""
authentication = ""

for opt, arg in opts:
    try:
        if opt in ("-h", "--host"):
            host = arg
        elif opt in ("-p", "--port"):
            port = int(arg)
        elif opt == "--password":
            password = arg
    except ValueError:
        err = True

if err or host is None:
    print("Usage: {scrname} -h host -p port".format(scrname=sys.argv[0]))
    print("Required argument:")
    print("  -h host, --host=host      host name on which to listen")
    print("Optional argument:")
    print("  -p port, --port=port      host port (numeric value)")
    print("  --password=password       password string")
    sys.exit(2)


def random_string(length: int) -> bytes:
    """Generates a random string of given length, encoded in base64.
    
    Parameters:
        length(int): string length (before base64 encoding)
    
    Returns:
        bytes: a random byte string
    """
    return base64.standard_b64encode(b"".join([chr(random.randint(0,127)).encode("utf-8") for n in range(length)]))


def new_client(client, server):
    """WebSocket server callback for when a new client connects.
    
    Parameters:
        client: client instance
        server: server instance
    """
    print("Client connected with ID {client_id}.".format(client_id=client["id"]))
    challenge = random_string(32)
    salt = random_string(32)
    
    data = {
        "op": 0,
        "d": {
            "obsWebSocketVersion": "5.0.0",
            "rpcVersion": 1,
            "authentication": {
              "challenge": challenge.decode("utf-8"),
              "salt": salt.decode("utf-8")
            }
          }
        }
    secret = base64.b64encode(hashlib.sha256(password.encode('utf-8') + salt).digest())
    global authentication
    authentication = base64.b64encode(hashlib.sha256(secret + challenge).digest()).decode('utf-8')
    server.send_message_to_all(json.dumps(data))


def message_received(client, server, message):
    """WebSocket server callback for when a message is received.
    
    Parameters:
        client: client instance
        server: server instance
        message: received message
    """
    print("Message from client {client_id}: {message}".format(client_id=client["id"], message=message))
    # test authentication
    try:
        js = json.loads(message)
        if "op" in js and js["op"]==1 and "d" in js and "authentication" in js["d"]:
            if js["d"]["authentication"] == authentication:
                print("Authentication succesful.")
            else:
                print("Authentication failed.")
    except:
        pass


def client_left(client, server):
    """WebSocket server callback for when a client disconnects.
    
    Parameters:
        client: client instance
        server: server instance
    """
    print("Client with ID {client_id} disconnected.".format(client_id=client["id"]))


print("Starting WebSocket server, listening to {host}:{port}".format(host=host,port=port))
server = WebsocketServer(host=host, port=port)
server.set_fn_new_client(new_client)
server.set_fn_message_received(message_received)
server.set_fn_client_left(client_left)
server.run_forever()
