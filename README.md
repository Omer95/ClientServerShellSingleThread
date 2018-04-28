# ClientServerShellSingleThread
A simple shell that executes certain arithmetic commands and linux programs through a network socket. There is a client program that accepts user commands and a server program that handles multiple clients and executes their commands. Most functions used in both programs are system APIs for linux. The programs are implemented as a single thread. I will upload a multithreaded client/server shell separately. 
The server program takes one argument for port number to listen for TCP/IP traffic.
The client program takes two arguments, the first is the server's address and the second is the port.
