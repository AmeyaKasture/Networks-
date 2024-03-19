#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int n;
int leavers=0;
int number=0;
FILE* file;

void solver(int client_id,int ex[],char vector[][1024],int id,int client_sockets[],int mode[]){
    char* msg ="POLL\n";
    if((send(client_id, msg, strlen(msg), 0)) == -1){
        printf("send_error");
        close(client_id);
        exit(1);
    }
    char command[1024];
    memset(command,0,1024);
    // printf("%s",command);
    if((recv(client_id, command, 1024, 0)) == -1){
        printf("recv");
        close(client_id);;
        exit(1);
    }
    // fprintf(file,"command:%s",command);
    // fflush(file);
    // printf("%s",command);
    if(strcmp(command,"NOOP\n")==0){
        fprintf(file,"%s: %s",vector[id],command);
        return;
    }
    else if(strcmp(command,"EXIT\n")==0){
        fprintf(file,"%s: %s",vector[id],command);
        fflush(file);
        leavers++;
        ex[id]=1;
        close(client_id);
        printf(file,"%sClient exited\n",vector[id]);
        fflush(file);
        return;
    }
    else if(strcmp(command,"LIST\n")==0){
        fprintf(file,"%s: %s",vector[id],command);
        fflush(file);
        char reply[1024];
        memset(reply,0,1024);
        for(int i =0;i<n;i++){
            if(mode[i]==1 && ex[i]!=1){
                strcat(reply,vector[i]);
                strcat(reply,":");
            }
        }
        for(int i =0;i<n;i++){
            if(mode[i]==0){
                strcat(reply,vector[i]);
                strcat(reply,":");
            }
        }
        reply[strlen(reply)-1]='\n';
        fprintf(file,"%s",reply);
        if((send(client_id, reply, 1024, 0)) == -1){
            printf("send_error");
            close(client_id);
            exit(1);
        }
        return;
    }
    else{
        if(command[strlen(command)-1]=='\n'){
            char *word;
            char *brkt;
            char reply[1024];
            memset(reply,0,1024);
            int j=0;
            const char* sep=":";
            const char* spc="|";
            char str1[1024];
            char* first_word,*second_word,third_word;
            first_word=strtok_r(command,sep,&brkt);
            if(strcmp(first_word,"MESG")==0){
                second_word=strtok_r(NULL,sep,&brkt);
                strcat(reply,vector[id]);
                strcat(reply,": ");
                strcat(reply,second_word);
                fprintf(file,"%s",reply);
                return;
            }
            else if(strcmp(first_word,"MSGC")==0){
                char* destination;
                destination=strtok_r(NULL,spc,&brkt);
                second_word=strtok_r(NULL,spc,&brkt);
                second_word[strlen(second_word)-1]='\0';
                char a[1024];
                memset(a,0,1024);
                strcat(a,"MSGC:");
                strcat(a,vector[id]);
                strcat(a,"|");
                strcat(a,second_word);
               
                int formatted_length = snprintf(NULL, 0, "|%02X\n", number) + 1; // Add 1 for the null terminator
                char tt[formatted_length];
                sprintf(tt, "|%02X\n", number);
                strcat(a, tt);
                int flag=0;
                for(int i=0;i<n;i++){
                    if(ex[i]==0 && strcmp(vector[i],destination)==0){
                        flag=1;
                        number++;
                        if((send(client_sockets[i], a, strlen(a), 0)) == -1){
                            printf("send_error");
                            close(client_id);
                            exit(1);
                        }
                        fprintf(file,"%s->%s:%02x:%s\n",vector[id],destination,number,second_word);
                        fflush(file);
                        return;
                    }
                }
                if(flag==0)fprintf(file,"INVALID RECIPIENT: %s\n",destination);
                return;

            }
        }
        printf("INVALID CMD\n");
        fprintf(file,"%s: INVALID CMD\n",vector[id]);
        return;
    }
    
}



// create connction
int create_connection(char* addr, int port) {
	int server_sockfd;
	struct sockaddr_in server_addrinfo;
    socklen_t sin_size;
    int yes = 1;
	if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("server: socket");
        exit(1);
    }
	if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        printf("setsockopt");
        exit(1);
    }
	if(listen(server_sockfd, n) == -1){
        printf("listen");
        close(server_sockfd);
        exit(1);
    }
	port = ntohs(server_addrinfo.sin_port);

	return server_sockfd;

}

// Accept incoming connections
int client_connect(int socket_id) {
	struct sockaddr_in client_addrinfo;
	int new_server_sockfd;
	socklen_t sin_size;
	sin_size = sizeof(client_addrinfo);
	//creation of socket for communication
	new_server_sockfd = accept(socket_id, (struct sockaddr*) &client_addrinfo, &sin_size);
    
	if(new_server_sockfd == -1){
        perror("accept");
        close(socket_id);
        exit(1);
    }
	return new_server_sockfd;

}


int main(int argc, char *argv[])
{
    if (argc != 4)
	{
		printf("Use 3 cli arguments\n");
		return -1;
	}
	n =atoi(argv[3]);
    file=fopen("log.txt","w");
    // printf("%x\n",n);
    int client_sockets[n];
    int ex[n];
    memset(ex,0,n);
    int mode[n];
    memset(mode,0,n);
	// extract the address and port from the command line arguments
	char addr[INET6_ADDRSTRLEN];
    unsigned int port;
	strcpy(addr, argv[1]);
    port = atoi(argv[2]);
    int socket_id = create_connection(addr, port);
    char vector[n][1024];
    for (int i = 0; i < n; i++) {
        memset(vector[i], 0, sizeof(vector[i]));
    }

    //First Connect
    for(int i=0;i<n;i++){
        int client_id = client_connect(socket_id);
        client_sockets[i]=client_id;
        char* msg ="NAME\n";
        if((send(client_id, msg, strlen(msg), 0)) == -1){
            perror("send_error");
            close(client_id);
            exit(1);
        }
        char reply[1024];
        memset(reply,0,1024);
        if((recv(client_id, reply, 1024, 0)) == -1){
            perror("recv");
            close(client_id);
            exit(1);
        }
        reply[strlen(reply)-1]='\0';
        
        strcpy(vector[i],reply);
        char* msgr ="MODE\n";
        if((send(client_id, msgr, strlen(msgr), 0)) == -1){
            perror("send_error");
            close(client_id);
            exit(1);
        }
        char rep[1024];
        memset(rep,0,1024);
        if((recv(client_id, rep, 1024, 0)) == -1){
            perror("recv");
            close(client_id);
            exit(1);
        }
        rep[strlen(rep)-1]='\0';
        // printf("%s\n",rep);
        fprintf(file,"CLIENT: %s; MODE: %s\n",reply,rep);
        if(strcmp(rep,"passive")==0){
            mode[i]=0;
            ex[i]=0;
            leavers++;
        }
        else{
            mode[i]=1;
            ex[i]=0;
        }
    }

    while(leavers!=n){
        for(int i=0;i<n;i++){
            if(ex[i]!=1 && mode[i]==1)solver(client_sockets[i],ex,vector,i,client_sockets,mode);
            // if(leavers==n-1)usleep(400);   
        }
    }
    for(int i=0;i<n;i++){
        if(mode[i]==0)close(client_sockets[i]);
    }
    printf("SERVER TERMINATED: EXITING......\n");
    fprintf(file,"SERVER TERMINATED: EXITING......\n");

    close(socket_id);
    return 0;    
}