#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>



typedef struct hd {
	int hid;
	char *msg;
}	ThreadData;

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client;


int clientCount = 0;
client *clients[5];
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
int uid = 1;


void str_overwrtite(){
	printf("\r%s",">");
	fflush(stdout);
}

void str_trim(char* arr, int len){
	for(int i = 0 ;i<len;i++){
		if(arr[i]=='\n'){
			arr[i]= '\0';
			break;
		}
	}
}

void addClient(client *cl){
	pthread_mutex_lock(&clientsMutex);
	for(int i = 0;i<5;i++){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}	
	
	pthread_mutex_unlock(&clientsMutex);
}

void removeClient( int uid){
	pthread_mutex_lock(&clientsMutex);
	for(int i = 0;i<5;i++){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}
	pthread_mutex_unlock(&clientsMutex);
}

void sendMessage(char *s, int uid){
	pthread_mutex_lock(&clientsMutex);
	for(int i = 0;i<5;i++){
		if(clients[i]){
			if(clients[i]->uid != uid){
				write(clients[i]->sockfd,s,strlen(s));
			}
		}
	}
	pthread_mutex_unlock(&clientsMutex);
}

void *manageClient(void *arg){
	char c[282];
	char name[32];
	int leave = 0;
	int first = 1;
	char buf[282] = " ha entrado al servidor!\n";
	
	clientCount++;
	client *cli = (client *)arg;
	
	bzero(c, 250);
	
	while(1){
		if(leave){
			break;
		}
		
		int receive = recv(cli->sockfd, c, 250, 0);
		if (receive > 0){
			if(strlen(c) > 0){
				if(first){
					strcpy(name, c);
					//printf("%s este es el nombre\n", name);
					sendMessage(strcat(c, buf), cli->uid);
					printf("%s", c);
					bzero(c, sizeof(c));
					str_trim(c, strlen(c));
					first = 0;
					
				}else{
					sendMessage(c, cli->uid);
					str_trim(c, strlen(c));
					printf("%s \n", c);
				}
			}
		} else if (receive == 0 || strcmp(c, "bye") == 0){
			sprintf(c, "%s ha abandonado el servidor.\n", name);
			printf("%s", c);
			sendMessage(c, cli->uid);
			leave = 1;
		} else {
			printf("ERROR: -1\n");
			leave = 1;
		}
		bzero(c, sizeof(c));
	}
	
	close(cli->sockfd);
	removeClient(cli->uid);
	free(cli);
	clientCount--;
	pthread_detach(pthread_self());
	
	return 0;
}

int main(int argc, char *argv[]){

    struct sockaddr_in serverAddress;
   
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[1]));
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(sockfd,(const struct sockaddr*)&serverAddress,sizeof(serverAddress));
    listen(sockfd,5);

	printf("---Bienvenido al chat---\n");
    while(1){
		
        struct sockaddr_in clientAddress;
        int clientSize = sizeof(clientAddress);
        int clientSocket = accept(sockfd,(struct sockaddr*)&clientAddress,(unsigned int*) &clientSize );
		
		if(clientCount+1>5){
			close(clientSocket);
		}
		
		pthread_t newThread;
		client *cli = (client *)malloc(sizeof(client));
		cli->address = clientAddress;
		cli->sockfd = clientSocket;
		cli->uid = uid++;
		
		addClient(cli);
		pthread_create(&newThread, NULL, &manageClient, (void*)cli);
    }
}