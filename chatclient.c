
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
extern int h_errno;

int sockfd = 0;
int leave = 0;
char name[32];


void str_trim(char* arr, int length) {
  for (int i = 0; i < length; i++) { // trim \n
    if (arr[i] == '>') {
      arr[i] = '\0';
      break;
    }
  }
}

void catchSignal(int sig){
	leave = 1;
}


void sendMsg(){
	char message[250] = {};
	char c[300] = {};
	
	while(1){
		printf("%s", "> ");
		fflush(stdout);
		fgets(message, 250, stdin);
		
		str_trim(message, 250);
		
		if(strcmp(message, "bye\n")==0){
			break;
		}else{
			sprintf(c, "%s > %s\n", name, message);
			send(sockfd, c, strlen(c)-1, 0);
		}
		
		bzero(message, 250);
		bzero(c, 300);
	}
	
	catchSignal(2);
}

void receiveMsg(){
	char message[250] ={};
	while(1){
		int r = recv(sockfd, message, 250, 0);
		if(r>0){
			printf("%s", message);
			printf("%s", "> ");
			fflush(stdout);
		}else if(r==0){
			break;
		}
		memset(message, 0, sizeof(message));
	}
}



int main(int argc, char *argv[]){
	
    char *hostName = argv[1];
    char c;
    struct sockaddr_in serverAddress;
    struct hostent *serverInfo = gethostbyname(hostName);

    bzero(&serverAddress,sizeof(serverAddress) );
	
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    //serverAddress.sin_addr = serverInfo->h_addr_list[0];
    //bcopy((char *)serverInfo->h_addr,
      //  (char*)&serverAddress.sin_addr.s_addr, serverInfo->h_length);
	
	signal(2, catchSignal);
	strcpy(name, argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//printf("hola\n");
	
    if(connect(sockfd, &serverAddress, sizeof(serverAddress))<0){
        printf("Error al conectar\n");
    }
	
	send(sockfd, name, 32, 0);
	
	printf("---Bienvenido al chat---\n");
	
	pthread_t sendThread;
	if(pthread_create(&sendThread, NULL, (void *)sendMsg, NULL)!=0){
		printf("Error creando thread para enviar");
		return 0;
	}
	   
	pthread_t receiveThread;
	if(pthread_create(&receiveThread, NULL, (void *)receiveMsg, NULL)!=0){
		printf("Error creando thread para recibir");
		return 0;
	}
	
	while(1){
		if(leave){
			printf("\nBye desde el servidor!\n");
			break;
		}
	}
	
	close(sockfd);
    return 0;
} 