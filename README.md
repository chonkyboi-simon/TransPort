# TransPort
TransPort is an application data bridge between two TCP ports, which allows you to hold/intercept the message sent between client and server. It is a simple man-in-the-middle tool that acts as a server for the client and a client for the server so that it can provide message editing while forwarding data between the server and the client under testing. The tool is developed in Ubuntu using Codeblocks, please open transport.cbp to import all the files in the project. Please install hex editor Bless (run: sudo apt-get install bless) as to edit the intercepted messages a hex editor is required. 

When transport is copied to a new machine, please run the followings commands to set it up.
1.	cd to the binary folder and make the transport program executable
sudo chmod +x transport
2.	transport needs bless hexeditor to edit the intercepted messages, run the following command to install bless hexeditor
sudo apt-get install bless

To try transport on a TLS connection:
Generate keys for the TLS server and client
openssl genrsa -out root.keypair.pem 2048
[option]openssl rsa -in root.keypair.pem -text -noout
openssl rsa -in root.keypair.pem -outform PEM -pubout -out root.public.pem
openssl req -x509 -new -nodes -key root.keypair.pem -sha256 -days 365 -out root.crt.pem -subj "/C=au/ST=vic/L=mel/O=org/OU=unit/CN=root"

openssl genrsa -out server.keypair.pem 2048
openssl req -new -key server.keypair.pem -out server.csr -subj "/C=au/ST=vic/L=mel/O=org/OU=unit/CN=server"
openssl x509 -req -in server.csr -CA root.crt.pem -CAkey root.keypair.pem -CAcreateserial -out server.crt.pem -days 365 -sha256

openssl genrsa -out client.keypair.pem 2048
openssl req -new -key client.keypair.pem -out client.csr -subj "/C=au/ST=vic/L=mel/O=org/OU=unit/CN=client"
openssl x509 -req -in client.csr -CA root.crt.pem -CAkey root.keypair.pem -CAcreateserial -out client.crt.pem -days 365 -sha256

server start
sudo openssl s_server -tls1_2 -accept 443 -verify 10 -verify_return_error -cert server.crt.pem -key server.keypair.pem -CApath . -CAfile root.crt.pem -msg -debug -tlsextdebug

client start (running on the same machine to connect to server at 127.0.0.1)
sudo openssl s_client -tls1_2 -verify 10 -verify_return_error -connect 127.0.0.1:443 -cert client.crt.pem -key client.keypair.pem -CApath . -CAfile root.crt.pem -msg -debug -tlsextdebug

To download the latest source, please check my github page: (I haven’t updated the program for ages…)
https://github.com/shiyu-zhou/TransPort
