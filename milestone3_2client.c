/*
* @author Omer Farooq Ahmed 
* version 5.0.0/milestone3.1
* this program is the client
* for the shell 
*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) { //takes server address parameters as arguments
	int sock, conn_status;
	struct sockaddr_in server;
	//create a socket
	sock=socket(AF_INET, SOCK_STREAM, 0);
	if (sock==-1) {perror("socket"); exit(1);}
	//set server information
	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	server.sin_addr.s_addr=inet_addr(argv[1]);
	//connect to server
	conn_status=connect(sock, (struct sockaddr *)&server, sizeof(server));
	if (conn_status==-1) {perror("connection"); exit(1);}
	
	while (1) {
		//take input arguments
		const char buff1[]="Please enter your command\n";
		write (STDOUT_FILENO, buff1, strlen(buff1));
		char buff2[100];
		int count=read (STDIN_FILENO, buff2, 100);
		buff2[count-1]='\0';
			
		//check to see if input is exit
		const char pexit[]="exit";
		if (strcmp(buff2, pexit)==0) {
			if (send(sock, buff2, count, 0)<0) {perror("send sock exit"); exit(1);}
			char msg[50];
			int msgcount=read(sock, msg, 50);
			write(STDOUT_FILENO, "\n", 1);
			write(STDOUT_FILENO, msg, msgcount);
			write(STDOUT_FILENO, "\n", 1);
			close(sock);
			break;
		}
		//send input to server
		if (send(sock, buff2, count, 0)==-1) {perror("send"); exit(1);}
		sleep(0.3);
		//read and display output from server
		char output[1024];
		int servc=read(sock, output, 1024);
		if (servc==-1) {perror("socket read"); exit(1);}
		write(STDOUT_FILENO, "\n", 1);
		write(STDOUT_FILENO, output, servc); 
		write(STDOUT_FILENO, "\n", 1);
	}
	//close(sock);
	return 0;
}
