import tkinter as tk
import threading

class UI:
    def __init__(self):
        self.root = tk.Tk()
        self.root.geometry("640x480")
        self.init_frames()
        
    def loop_listen_server_fn(self):
        self.listen_server_fn()
        
        
    def run(self):
        th = threading.Thread(target=self.loop_listen_server_fn)
        th.start()
        self.root.mainloop()

    def init_frames(self):
        self.channels = tk.Frame(self.root, width=128, background="green").pack(
            anchor="w", fill="y", expand=False, side="left"
        )

        self.chat_window = tk.Frame(self.root, background="red").pack(
            anchor="n", fill="both", expand=False
        )
        self.message_history = tk.StringVar()
        self.messages = tk.Label(
            self.chat_window, background="orange", textvariable=self.message_history,
            anchor="nw",
            justify="left"
        
        )
        self.messages.pack(
            anchor="n", fill="both", expand=True, side="top"
        )

        self.input_frame = tk.Frame(self.chat_window, background="blue").pack(
            anchor="n", fill="x", expand=False, side="top"
        )

        self.send_message_btn = tk.Button(self.input_frame, text="Send", command=self.send_message).pack(
            fill="y", expand=False, side="right"
        )

        self.send_message_textbox = tk.Text(self.input_frame, height=4)
        self.send_message_textbox.pack(
            anchor="e", fill="both", expand=True, side="right"
        )

    def bind_send_message_fn(self, fn):
        self.send_message_fn = fn

    def bind_listen_server_fn(self, fn):
        self.listen_server_fn = fn

    def send_message(self):
        text = self.send_message_textbox.get("1.0",'end-1c')
       

        self.send_message_fn(text)
        self.send_message_textbox.delete("1.0", "end-1c")

    def add_message(self, msg):
        self.message_history.set(msg)
