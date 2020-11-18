!) Alejandro Marcovich

2) After compling the client and server files use the provided my_config.txt to run with client. Like so, "./client my_config.txt". Make sure the server is running as well before running client or standalone. Standalone uses server to send it's UDP packets but doesn't rely on it for anything else.

compiling:
gcc -o client client.c
gcc -o server server.c
gcc -o standalone standalone.c