#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

FILE* file;
char mode[1024];
// Create a connection to the server
int create_connection(char* addr, int port) {
	int client_sockfd;
	if((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("client: socket");
		exit(1);
	}
	struct sockaddr_in server_addrinfo, client_addrinfo;
	server_addrinfo.sin_family = AF_INET;
    server_addrinfo.sin_port = htons(port);
	char client_IP[INET6_ADDRSTRLEN];
    unsigned int client_port;
	
	//Checking if there exists a server with that address
    if (inet_pton(AF_INET, addr, &server_addrinfo.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        close(client_sockfd);
        exit(1);
    }

	// 2. CONNECT
	// client connects if server port has started listen()ing and queue is non-full; however server connects to client only when it accept()s
    if(connect(client_sockfd, (struct sockaddr*)&server_addrinfo, sizeof(server_addrinfo)) == -1){ 
        printf("Could not find server");
        close(client_sockfd);
        exit(1);
    }

	memset(&client_addrinfo, 0, sizeof(client_addrinfo));
    socklen_t len = sizeof(client_addrinfo);
    getsockname(client_sockfd, (struct sockaddr*) &client_addrinfo, &len);
    
	if(inet_ntop(client_addrinfo.sin_family, &client_addrinfo.sin_addr, client_IP, sizeof(client_IP)) <= 0){
        printf("\nAddress Conversion Error\n");
        close(client_sockfd);
        exit(1);
    }
    client_port = ntohs(client_addrinfo.sin_port);

	return client_sockfd;
	
}

void send_data(int socket_id) {
    int send_count;
	char msg[1024];
	memset(msg, 0, 1024);
	

	// fgets(msg, 1024, stdin);
	fgets(msg,1024,stdin);
	
	 // 3. SEND
	if(strcmp(msg, "EXIT\n") == 0){
            if((send_count = send(socket_id, msg, strlen(msg), 0)) == -1){
                perror("send");
                close(socket_id);
                exit(1);
            }
			printf("CLIENT TERMINATED: EXITING......\n");
            exit(1);
    }

	else if(strcmp(msg, "LIST\n") == 0){
            if((send_count = send(socket_id, msg, strlen(msg), 0)) == -1){
                perror("send");
                close(socket_id);
                exit(1);
            }
			int recv_count;
			char reply[1024];
			memset(reply, 0, 1024);
			if((recv_count = recv(socket_id, reply, 1024, 0)) == -1){
					perror("recv");
					exit(1);
			}
			reply[strlen(reply)-1]='\0';
			char *brkt;
			const char* sep=":";
            char str1[1024];
            char* first_word;
			int i=1;
            for(first_word=strtok_r(reply,sep,&brkt);first_word;first_word=strtok_r(NULL,sep,&brkt),i++){
				printf("%d. %s\n",i,first_word);
			}
			return;
	// file=fopen("client
    }
	if((send_count = send(socket_id, msg, strlen(msg), 0)) == -1){
		perror("send");
		close(socket_id);
		exit(1);
	}
	// fputs(msg,file);


}

// Receive input from the server
void recv_data(int socket_id) {
	int recv_count;
	char reply[1024];
	memset(reply, 0, 1024);
	if((recv_count = recv(socket_id, reply, 1024, 0)) == 0){
            // perror("recv");
            exit(1);
    }

	// file=fopen("client_file.txt","r+");
	// fseek(file,0,SEEK_END);
    if(strcmp(reply,"POLL\n")==0){
        printf("ENTER CMD: ");
        return;
    }
	else{
        // printf("%s\n",reply);
        char* first_word,*second_word,*message,*number,*brkt;
        first_word=strtok_r(reply,":",&brkt);
        second_word=strtok_r(NULL,"|",&brkt);
        message=strtok_r(NULL,"|",&brkt);
        number=strtok_r(NULL,"|",&brkt);
        number[strlen(number)-1]='\0';
        printf("%s:%s:%s\n",second_word,number,message);

        recv_data(socket_id);
    }


}

int main(int argc, char *argv[])
{
    if (argc != 5)
	{
		printf("Use 4 cli arguments\n");
		return -1;
	}
    char username[1024];
    memset(username,0,1024);
    strcpy(username,argv[3]);
	username[strlen(username)]='\n';
   
    memset(mode,0,1024);
    strcpy(mode,argv[4]);
    mode[strlen(mode)]='\n';
    
	// extract the address and port from the command line arguments
	char addr[INET6_ADDRSTRLEN];
    unsigned int port;
	strcpy(addr, argv[1]);
    port = atoi(argv[2]);
	int socket_id = create_connection(addr, port);
    int recv_count;
	char reply[1024];
	memset(reply, 0, 1024);
	if((recv_count = recv(socket_id, reply, 1024, 0)) == -1){ 
            perror("recv");
            exit(1);
    }
    printf("INITIALIZING......\n");
    if((send(socket_id, username, strlen(username), 0)) == -1){
		perror("send");
		close(socket_id);
		exit(1);
	}
    memset(reply,0,1024);
    if((recv_count = recv(socket_id, reply, 1024, 0)) == -1){ 
            perror("recv");
            exit(1);
    }
    if((send(socket_id, mode, strlen(mode), 0)) == -1){
		perror("send");
		close(socket_id);
		exit(1);
	}
	while (1)
    {
        recv_data(socket_id);
        send_data(socket_id);
    }
}
