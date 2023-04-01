# Computer_Networks
This repo contains code snippets for socket programming in C done as part of Computer Networks labs (CS F303)

## Requirements
- OS: Ubuntu 20.04 or above

## Steps to execute (on Ubuntu 20.04)
- Suppose the files are `client.c` and `server.c`, follow the below steps:
  - Step 1: Get your system IP by the following command:
  For Ubuntu on WSL:
  ```
  hostname -I
  ```
  - Step 2: Put this value in the variable named `server_ip` in `client.c`
  - Step 3: Compile both the files using:
    ```
    gcc -o client client.c
    gcc -o server server.c
    ```
  - Step 4: Open two terminals (one for server and one for client). In the first one run server as:
  ```
  ./server
  ```
  On the other one run client as:
  ```
  ./client
  ```
  and follow the prompts
