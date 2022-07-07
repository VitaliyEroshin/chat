from src.sockets import *
from src.ui import *
import threading
import time

session()
ui = UI()

def listen_to_server():
    chat_history = ""
    while True:
        obj = read_server()

        
        if obj.object_type == "text":
            chat_history += obj.content + "\n"
            ui.add_message(chat_history)

ui.bind_send_message_fn(send_message)
ui.bind_listen_server_fn(listen_to_server)

ui.run()
sock.close()

