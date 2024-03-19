#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024
#define MAX_USERNAME_LEN 1024

int socket_id;
char username[1024];

void *send_thread(void *arg) {
    fgets(username,1024,stdin);
    username[strlen(username)-1]='\0';
    if (send(socket_id, username, 1024, 0) == -1) {
            perror("send");
            exit(1);
    }
    if(username[strlen(username)-1]=='r'){
        char password[1024];
        memset(password,0,1024);
        fgets(password,1024,stdin);
        password[strlen(password)-1]='\0';
        if (send(socket_id, password, strlen(password), 0) == -1) {
            perror("send");
            exit(1);
        }
    }
    char message[MAX_MSG_LEN];
    while (1) {
        memset(message,0,MAX_MSG_LEN);
        fgets(message, MAX_MSG_LEN, stdin);
        if (send(socket_id, message, strlen(message), 0) == -1) {
            perror("send");
            exit(1);
        }
    }
    return NULL;
}

void *recv_thread(void *arg) {
    char message[MAX_MSG_LEN];
    while (1) {
        memset(message,0,MAX_MSG_LEN);
        int recv_count = recv(socket_id, message, MAX_MSG_LEN, 0);
        if (recv_count == -1) {
            perror("recv");
            exit(1);
        } else if (recv_count == 0) {
            printf("Connection closed by server\n");
            exit(0);
        } else {
            message[recv_count] = '\0';
            printf("%s", message);
        }
    }
    return NULL;
}

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

int main(int argc, char *argv[]) {
     if (argc != 3)
	{
		printf("Use 2 cli arguments\n");
		return -1;
	}
    memset(username,0,1024);
  
	// extract the address and port from the command line arguments
	char addr[INET6_ADDRSTRLEN];
    unsigned int port;
	strcpy(addr, argv[1]);
    port = atoi(argv[2]);
	socket_id = create_connection(addr, port);


    pthread_t send_tid, recv_tid;
    if (pthread_create(&send_tid, NULL, send_thread, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }
    if (pthread_create(&recv_tid, NULL, recv_thread, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }

    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);

    close(socket_id);
    return 0;
}