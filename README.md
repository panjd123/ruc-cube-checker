# RUC-cube-checker

非常直白的系统，上传魔方和步骤，调用助教当年写的魔方转一下，输出结果。

## 怎么部署

非常草台的部署方式：

```bash
g++ cube.cpp -o cube -O2
pip install flask
python app.py --host 0.0.0.0 --port 8123
```

如果你需要一个 WSGI 服务器，可以使用 `gevent`：

```bash
pip install gevent
python app.py --host 0.0.0.0 --port 8123 --wsgi
```

~~快速上线~~

```bash
nohup python app.py --host 0.0.0.0 --port 8123 --wsgi &
```

## 其他可能有用的东西

### systemd 服务文件

```ini
[Unit]
Description=RUC-cube-checker
After=network.target

[Service]
Type=simple
User=jarden
Group=jarden
WorkingDirectory=/home/jarden/RUC-cube-checker
ExecStart=/home/jarden/anaconda3/bin/python /home/jarden/RUC-cube-checker/app.py --host 0.0.0.0 --port 8123 --wsgi
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

### Nginx 配置文件

```nginx
server {
    listen 80;
    server_name cube.ruc.panjd.net;
    return 301 https://$host$request_uri;
}

server {
    listen 443 ssl;
    server_name cube.ruc.panjd.net;

    # include ssl.conf; # SSL 配置文件
    ssl_certificate /home/jarden/www/cert/ruc.panjd.net_ecc/fullchain.cer;
    ssl_certificate_key /home/jarden/www/cert/ruc.panjd.net_ecc/ruc.panjd.net.key;


    location / {
        real_ip_header X-Real-IP;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header Host $http_host;
        proxy_redirect off;
        proxy_pass http://127.0.0.1:8123;
    }
}
```