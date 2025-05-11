# Team Cipher CS564 Capstone Project
Presentation: https://docs.google.com/presentation/d/1wZEYjZx_0QHwhiwaYdVVf7jkP-cUIb2kz7qhborW6UM/edit?usp=sharing

# What we did
- Exploited CVE-2017-5638 in Apache Struts2
- Delivered a custom C implant to victim via RCE
- Used Base64+bash trick to launch the payload
- Built a Python-based TCP C2 server for remote tasking and data exfiltration

# Protocol: TCP Listener (Command & Control)
- Standard TCP communication over socket (3-way handshake)
- Persistent connection: Listener waits for implant to connect back
- Tasking: Attacker sends commands (e.g., shell commands, file retrieval)
- Exfiltration: Implant sends outputs and stolen files via TCP
- Encrypted payloads (e.g., AES-256 or XOR-encoded data chunks)

# Vulnerable Setup
### Docker Setup:
- Image: medicean/vulapps:s_struts2_s2-045
- Runs vulnerable web app on port 8080 (mapped to host port 80)
- Exploit URL: http://localhost/memocreate.action
### Attacker Environment:
- Windows Host with WSL and PowerShell
- PowerShell: runs c2_server.py, http.server
- WSL: compiles and hosts implant_client.c
### Config File:
- .env used for IP, port, and obfuscation key

# Implant Development
Script: implant_client.c: Script for implant that acts like malware. It connects to C2 server, 
waits for commands, executes them on the victim machine, and sends back the results.

Behaviors:
- It reads the C2 server's IP and port from a .env file.
- Opens a TCP socket to the server.
- Listens for incoming command strings.
- Uses popen() to execute the commands on the system.
- Captures the output and sends it back over the socket.

Compiled in WSL: gcc -o implant -O3 -fno-stack-protector -z execstack -static implant_client.c

