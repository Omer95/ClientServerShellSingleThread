/*
* @author Omer Farooq Ahmed 
* version 6.0.1/milestone3.2
* this program is the shell server
* that serves multiple clients
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>              
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

//NODE for process list
typedef struct process {
	char name[30];
	pid_t pid;
	char status[30];
	struct process *next;
} node;

//Print all the NODEs in the pocess list
char * print_list(node *head) {
	const char newline[]="\n";
	const char tab[]="\t";
	char *buff=malloc(2000);
	strcpy(buff, "\nNAME\tPID\tSTATUS\n");
	//write  (STDOUT_FILENO, buff, strlen(buff));
	node *current=head;
	while (current!=NULL) {
		//write (STDOUT_FILENO, current->name, strlen(current->name));
		strcat(buff, current->name);
		//write (STDOUT_FILENO, tab, strlen(tab));
		strcat(buff, "\t");
		char id[20];
		int count=sprintf(id, "%d", current->pid);
		//write (STDOUT_FILENO, id, count);
		strcat(buff, id);
		//write (STDOUT_FILENO, tab, strlen(tab));
		strcat(buff, "\t");
		//write (STDOUT_FILENO, current->status, strlen(current->status));
		strcat(buff, current->status);
		//write (STDOUT_FILENO, newline, strlen(newline));
		strcat(buff, "\n");
		current=current->next;
	}
	return buff;
}

//Print all the Active NODEs
char * print_active(node *head) {
	const char newline[]="\n";
	const char tab[]="\t";
	char *buff=malloc(2000);
	strcpy(buff, "\nNAME\tPID\tSTATUS\n");
	//write  (STDOUT_FILENO, buff, strlen(buff));
	node *current=head;
	while (current!=NULL) {
		if (strcmp(current->status, "Active")==0) {
			//write (STDOUT_FILENO, current->name, strlen(current->name));
			strcat(buff, current->name);
			//write (STDOUT_FILENO, tab, strlen(tab));
			strcat(buff, "\t");
			char id[20];
			int count=sprintf(id, "%d", current->pid);
			//write (STDOUT_FILENO, id, count);
			strcat(buff, id);
			//write (STDOUT_FILENO, tab, strlen(tab));
			strcat(buff, "\t");
			//write (STDOUT_FILENO, current->status, strlen(current->status));
			strcat(buff, current->status);
			//write (STDOUT_FILENO, newline, strlen(newline));
			strcat(buff, "\n");
		}
		current=current->next;
	}
	return buff;
}

//Add a NODE to the end of the list
void push_list(node *head, char *aname, pid_t apid, char *astatus) {
	node *current=head;
	while (current->next!=NULL) {
		current=current->next;
	}	
	current->next=malloc(sizeof(node));
	strcpy(current->next->name, aname);
	//current->next->name=aname;
	//current->next->name[strlen(aname)-1]='\0';
	current->next->pid=apid;
	strcpy(current->next->status, astatus);
	current->next->next=NULL;
}

//Remove a NODE from the end of the list
void pop_node(node *head) {
	node *current=head;
	while (current->next->next!=NULL) {
		current=current->next;
	}
	current->next=NULL;
}

//Change Node status
void kill_node (node *head, int id, char *signal) {
	node *current=head;
	while (current->pid!=id) {
		current=current->next;
	}
	strcpy(current->status, signal);
}

int childid;
int status;
int valid=0;
//handling SIGCHLD
void handler1(int signum) {
	childid=waitpid(-1, &status, WNOHANG);
	if (childid>0) {
		valid++;
	}
}

const char newline[]="\n";
int main(int argc, char *argv[]) {
	//Node assignment
	node *head=NULL;
	head= malloc(sizeof(node));
	const char parent[]="Shell";
	const char shellstatus[]="Active";
	strcpy(head->name, parent);
	head->pid= getpid();
	strcpy(head->status, shellstatus);
	head->next=NULL;
	//set SIGCHLD signal 
	__sighandler_t sigval=signal(SIGCHLD, handler1);
	if (sigval==SIG_ERR){
		perror("signal");
	}
	//SETUP SERVER
	int serv_sock, client_sock;
	struct sockaddr_in server;
	char buff[1024];
	int incount;
	pid_t cpid;
	//create socket
	serv_sock=socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sock==-1) {perror("socket"); exit(1);}
	//set socket information
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(atoi(argv[1]));
	//bind the socket to server's address
	if (bind(serv_sock, (struct sockaddr *)&server, sizeof(server))==-1) {
		perror("binding1");
		exit(1);
	}
	//listen for connections
	if (listen(serv_sock, 5)==-1) {perror("listen"); exit(1);}
	
	while (1) {
		//accept connections
		if ((client_sock=accept(serv_sock, 0, 0))==-1) {
			perror("accept");
			exit(1);
		}
		//fork into new child
		cpid=fork();
		if (cpid==-1) {perror("fork"); exit(1);}
		if (cpid==0) {
			while (1) {
				char *list[30]; //This creates an array of character pointers (strings)
				//read arguments from socket
				char inbuff[100];
				if (read(client_sock, inbuff, 100)==-1) {perror("socket read"); exit(1);}
				// Begin tokenization and entering tokens in list
				const char delim[]=" ";
				char *token;
				token=strtok(inbuff, delim);
				const char newline[]="\n";
				int i=0;
				while (token!= NULL) {
					list[i]=token;
					i++;
					token=strtok(NULL,delim);
				}
				//check for exit
				const char pexit[]="exit";
				if (strcmp(list[0], pexit)==0) {
					send(client_sock, "disconnected", 12, 0);
					break;
				}
				// Begin Addition operation
				const char add[]="add";
				if (strcmp(list[0], add)==0) {
					int result=0;
					for (int j=1; j<i; j++) {
						result+=atoi(list[j]);
					}
					char sum[50];
					int sumcount=sprintf(sum, "%d", result);
					//sum[sumcount-1]='\0';
					if (send(client_sock, sum, sumcount+1, 0)==-1) {perror("socket send add"); exit(EXIT_FAILURE);}	
				}
				// Begin Subtraction operation
				const char sub[]="sub";
				if (strcmp(list[0], sub)==0) {
					int result=0;
					if (i!=1) 
						result+=atoi(list[1]);
					for (int j=2; j<i; j++) {
						result-=atoi(list[j]);
					}
					char sum[50];
					int sumcount=sprintf(sum, "%d", result);
					if (send(client_sock, sum, sumcount+1, 0)==-1) {perror("socket send sub"); exit(EXIT_FAILURE);}
				}
				// Begin Multiply operation	
				const char mul[]="mul";
				if (strcmp(list[0], mul)==0) {
					int result=1;
					for (int j=1; j<i; j++) {
						result=result*atoi(list[j]);
					}
					char product[50];
					int productcount=sprintf(product, "%d", result);
					if (send(client_sock, product, productcount+1, 0)==-1) {perror("socket send mul"); exit(EXIT_FAILURE);}
				}
				// Begin Division operation
				const char div[]="div";
				if (strcmp(list[0], div)==0) {
					int result;
					if (i>1)
						result=atoi(list[1]);
					for (int j=2; j<i; j++) {
						result=result/atoi(list[j]);
					}
					char product[50];
					int productcount=sprintf(product, "%d", result);
					if (send(client_sock, product, productcount+1, 0)==-1) {perror("socket send div"); exit(EXIT_FAILURE);}
				}
				// Begin Exec operation
			
				const char run[]="run";
				if (strcmp(list[0], run)==0) {
					if (i>1) {
						char *file=list[1];
						char *args[i];
						char astatus[]="Active";
						for (int j=0; j<i-1; j++) {
							args[j]=list[j+1];
						}
						args[i-1]=NULL;
						//create pipe for parent child communication
						int pipe3[2];
						if (pipe(pipe3)==-1) {perror("pipe3: "); exit(EXIT_FAILURE);}
						fcntl(pipe3[1], F_SETFD, FD_CLOEXEC);
						fcntl(pipe3[0], F_SETFD, FD_CLOEXEC);
						//fork to create new application process
						int cpid= fork();
						if (cpid==-1) {perror("fork: "); exit(0);}
						//child process
						if (cpid==0) {
							//close reading end of pipe
							close(pipe3[0]);
							pid_t pid= getpid();
							int err=execvp(file, args);
							if (err==-1) {
								perror("exec: ");
								if (write(pipe3[1], "FAILED\n", 7)==-1) {perror("pipe3 write "); exit(EXIT_FAILURE);}
								send (client_sock, "failed written\n", 15, 0);
								
							}
						}
						//parent process
						if (cpid!=0) {
							//close writing end of pipe
							close(pipe3[1]);
							sleep(0.4);
							char in[10];
							int chldcount=read(pipe3[0], in, 10);
							if (chldcount==0) { //successfuly read from a closed pipe
								//adding the new process to the process list
								push_list(head, file, cpid, astatus);
								if (send(client_sock, "execution complete\0", 19, 0)==-1) {perror("send run"); exit(EXIT_FAILURE);}
							}
						}
				
					}
					//if (send(client_sock, "no arguments for run\0", 21, 0)==-1) {perror("send run"); exit(EXIT_FAILURE);}
				}
				// List all operation
				const char palist[]="list-all";
				if (strcmp(list[0], palist)==0) {
					if (valid>0) {
						char term[]="Terminated";
						kill_node(head, childid, term);
						valid=0;
					}
			
					char *output=print_list(head);
					if (send(client_sock, output, strlen(output)+1, 0)==-1) {perror("sock list all"); exit(EXIT_FAILURE);}
				}
				// List operation
				const char plist[]="list";
				if (strcmp(list[0], plist)==0) {
					if (valid>0) {
						char term[]="Terminated";
						kill_node(head, childid, term);
						valid=0;
					}
				
					//print_active(head);
					char *output=print_active(head);
					if (send(client_sock, output, strlen(output)+1, 0)==-1) {perror("sock list"); exit(EXIT_FAILURE);}
				}
				// Help operation
		
				const char help[]="help";
				if (strcmp(list[0], help)==0) {
					char helpbuff[]="\nWelcome to Omer's shell version 1.0\nchose from the following commands:	\n1. Add (add 1 2 ...)\n2. Subtract (sub 10 9 ...)\n3. Multiply (mul 1 2 3 ...)\n4. Divide (div 100 10 ...)\n5. Run (run gedit example.txt ...)\n6. List Running Processes (list)\n7. List All Processes (list-all)\n8. Kill (kill [process id] [signal]/ kill [process id])\n9. Exit (exit)\n";
					if (send(client_sock, helpbuff, strlen(helpbuff)+1, 0)==-1) {perror("sock help"); exit(EXIT_FAILURE);}
				}
				// Kill operation
				const char pkill[]="kill";
				if (strcmp(list[0], pkill)==0) {
					if (i==3 && list[1]>0) {
						int errno= kill (atoi(list[1]), atoi(list[2]));
						if (errno==-1) {perror ("kill:");}
						kill_node(head, atoi(list[1]), list[2]);
					} 
					if (i==2 && list[1]>0) {
						int errno= kill (atoi(list[1]), SIGTERM);
						if (errno==-1) {perror ("kill:");}
						char term[]="Terminated";
						kill_node(head, atoi(list[1]), term);
					}
					if (send(client_sock, "kill successful\0", 16, 0)==-1) {perror("sock kill"); exit(EXIT_FAILURE);}
				}
				if (strcmp(list[0], pkill)!=0 && strcmp(list[0], help)!=0 && strcmp(list[0], plist)!=0 && strcmp(list[0], palist)!=0 && strcmp(list[0], run)!=0 && strcmp(list[0], div)!=0 && strcmp(list[0], mul)!=0 && strcmp(list[0], sub)!=0 && strcmp(list[0], add)!=0) { 
					if (send(client_sock, "illegal command\0", 16, 0)==-1) {perror("sock illegal"); exit(EXIT_FAILURE);}
				}
			}
		}
		
	}
}
