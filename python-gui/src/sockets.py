import socket
from src.encoder import Object, Encoder
enc = Encoder()
sock = socket.socket()
print_message_fn = None

def make_auth_attempt(username, password):
    attempt = Object()
    attempt.object_type = "loginAttempt"

    attempt.content += username
    attempt.content += chr(1)
    attempt.content += password
    return attempt

def read_server():
    size_raw = sock.recv(2).decode("utf-8")
    fi, se = ord(size_raw[0]), ord(size_raw[1])
    modulo = 127
    size = fi + se * modulo

    data = sock.recv(size)
    obj = enc.decode(data)
    return obj

def send_data(data):
    size = len(data)

    modulo = 127
    fi, se = size % modulo, size // modulo
    s = chr(fi) + chr(se)
    sock.send(bytes(s, "utf-8"))
    sock.send(data)

def auth():
    send_data(enc.encode(make_auth_attempt("admin", "admin")))
    obj = read_server()
    if obj.get_value("returnCode") == 0:
        print("OK, authorized.")
    else:
        print("error when authorized")

def send_message(text):
    if (text == ""):
        return
    obj = Object()
    obj.object_type = "text"
    obj.content = text
    print("sending...")
    send_data(enc.encode(obj))

def session():
    sock.connect(('localhost', 8888))
    auth()
    
