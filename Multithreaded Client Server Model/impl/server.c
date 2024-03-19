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

#define MAX_CLIENTS 1024 // Maximum number of clients the server can handle simultaneously

int n;
int group_counter;
int leavers = 0;
int number = 0;
FILE *file;
int client_sockets[MAX_CLIENTS];
int ex[MAX_CLIENTS];
char vector[MAX_CLIENTS][1024];
int socket_id;
char vector[MAX_CLIENTS][1024];
char history[MAX_CLIENTS][1024];
int counter;
int arrivals[1024];
int types[1024];
char password[1024];
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t exit_mutex= PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int index;
} ThreadArgs;

typedef struct{
    char groupname[1024];
    int users[1024];
    int total_users;
}Group;

Group groups[1024];

int create_connection(char* addr, int port) {
    int server_sockfd;
    struct sockaddr_in server_addrinfo;
    int yes = 1;

    // Create socket
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Set socket options
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        close(server_sockfd);
        exit(1);
    }

    // Initialize server address structure
    memset(&server_addrinfo, 0, sizeof(server_addrinfo));
    server_addrinfo.sin_family = AF_INET;
    server_addrinfo.sin_addr.s_addr = INADDR_ANY;
    server_addrinfo.sin_port = htons(port);

    // Bind the socket to a specific address and port
    if (bind(server_sockfd, (struct sockaddr*)&server_addrinfo, sizeof(server_addrinfo)) == -1) {
        perror("bind");
        close(server_sockfd);
        exit(1);
    }

    // Start listening for incoming connections
    if (listen(server_sockfd, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_sockfd);
        exit(1);
    }

    return server_sockfd;
}


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


void solver(int id,char command[],int temp){
    int client_id=client_sockets[id];
    
    if(strcmp(command,"NOOP\n")==0){
        // fprintf(file,"%s: %s",vector[id],command);
        return;
    }
    else if(strcmp(command,"EXIT\n")==0){
        // fprintf(file,"%s: %s",vector[id],command);
        // fflush(file);
        leavers++;
        ex[id]=1;
        close(client_id);
        for(int i=0;i<n;i++){
            if(ex[i]==0){
                solver(i,"LIST\n",0);
            }
        }
        // printf(file,"%sClient exited\n",vector[id]);
        // fflush(file);
        return;
    }
    else if(strcmp(command,"LIST\n")==0){
        // fprintf(file,"%s: %s",vector[id],command);
        // fflush(file);
        char reply[1024];
        memset(reply,0,1024);
        strcat(reply,"LIST-");
        for(int i =0;i<n;i++){
            if( ex[i]!=1){
                strcat(reply,vector[i]);
                strcat(reply,"|");
                char ans[2];
                memset(ans,0,2);
                if(types[i]==0)strcpy(ans,"n");
                else strcpy(ans,"r");
                strcat(reply,ans);
                strcat(reply,":");
            }
        }
        reply[strlen(reply)-1]='\n';
        // fprintf(file,"%s",reply);
        if((send(client_id, reply, 1024, 0)) == -1){
            printf("send_error");
            close(client_id);
            exit(1);
        }
        return;
    }
    else if(strcmp(command,"HIST\n")==0){
        // fprintf(file,"%s: %s",vector[id],command);
        // fflush(file);
        
        char reply[1024];
        memset(reply,0,1024);
        for(int i =0;i<=temp;i++){
                strcat(reply,history[i]);
        }
        // fprintf(file,"%s",reply);
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
                // fprintf(file,"%s",reply);
                return;
            }
            else if(strcmp(first_word,"MSGC")==0){
                char* destination;
                destination=strtok_r(NULL,sep,&brkt);
                second_word=strtok_r(NULL,sep,&brkt);
                // second_word[strlen(second_word)-1]='\0';
                char a[1024];
                memset(a,0,1024);
                sprintf(a,"%s:%s",vector[id],second_word);
                int flag=0;
                for(int i=0;i<n;i++){
                    if(ex[i]==0 && strcmp(vector[i],destination)==0){
                        flag=1;
                        // number++;
                        if((send(client_sockets[i], a, strlen(a), 0)) == -1){
                            ex[client_sockets[i]]=1;
                        }
                        // fprintf(file,"%s->%s:%02x:%s\n",vector[id],destination,number,second_word);
                        // fflush(file);
                        break;
                    }
                }
                if(flag==0){
                    strcpy(a,"USER NOT FOUND\n");
                    if((send(client_sockets[id], a, 1024, 0)) == -1){
                            printf("send_error");
                            close(client_id);
                            exit(1);
                        }
                }
                return;
            }
            else if(strcmp(first_word,"BCST")==0){
                second_word=strtok_r(NULL,sep,&brkt);
                char a[1024];
                memset(a,0,1024);
                sprintf(a,"%s:%s",vector[id],second_word);
                for(int i=0;i<n;i++){
                    if(ex[i]==0 && i!=id){
                        if((send(client_sockets[i], a, strlen(a), 0)) == -1){
                            // fprintf(file,"send_error in bsct\n\n\n\n\n\n");
                            ex[client_sockets[i]]=1;
                        }
                        // fprintf(file,"%s->%s:%02x:%s\n",vector[id],destination,number,second_word);
                        // fflush(file);
                    }
                }
                return;
            }
            else if(strcmp(first_word,"CAST")==0){
                second_word=strtok_r(NULL,sep,&brkt);
                char a[1024];
                memset(a,0,1024);
                int gt=types[id];
                sprintf(a,"%s:%s",vector[id],second_word);
                for(int i=0;i<n;i++){
                    if(ex[i]==0 && i!=id && types[i]==gt){
                        if((send(client_sockets[i], a, strlen(a), 0)) == -1){
                            // fprintf(file,"send_error in cast\n\n\n\n\n\n");
                            ex[client_sockets[i]]=1;
                        }
                        // fprintf(file,"%s->%s:%02x:%s\n",vector[id],destination,number,second_word);
                        // fflush(file);
                    }
                }
                return;
            }
            else if(strcmp(first_word,"GRPS")==0){
                char users[1025][1025];
                char* groupname,*str2,*subtoken,*saveptr2;
                for(int i=0;i<1024;i++)memset(users[i],0,1024);
                second_word=strtok_r(NULL,sep,&brkt);
                int tc=0;
                for (str2 = second_word; ; str2 = NULL,tc++) {
                    subtoken = strtok_r(str2, ",", &saveptr2);
                    if (subtoken == NULL)break;
    
                    strcpy(users[tc],subtoken);
               }
               int final[1024];
               memset(final,0,1024);
               for(int i=0;i<tc;i++){
                    int flag=0;
                    for(int j=0;j<n;j++){
                        if(ex[j]==0 && strcmp(users[i],vector[j])==0){
                            final[i]=j;
                            flag=1;
                        }
                    }
                    if(!flag){
                        char* ans="INVALID USERS LIST\n";
                        if((send(client_id, ans, strlen(ans), 0)) == -1){
                            
                        }
                        return;
                    }
                }
                groupname=strtok_r(NULL,sep,&brkt);
                groupname[strlen(groupname)-1]='\0';
                for(int i=0;i<=group_counter;i++){
                    if(strcmp(groups[i].groupname,groupname)==0){
                        groups[i].total_users=tc;
                        for(int j=0;j<tc;j++){
                            groups[i].users[j]=final[j];
                        }
                    }
                    char ans[1024];
                    memset(ans,0,1024);
                    sprintf(ans,"GROUP %s CREATED\n",groupname);
                    if((send(client_id, ans, strlen(ans), 0)) == -1){
                            printf("send_error");
                    }
                    return;

                }
                group_counter++;
                groups[group_counter].total_users=tc;
                strcpy(groups[group_counter].groupname,groupname);
                for(int j=0;j<tc;j++){
                    groups[group_counter].users[j]=final[j];
                }
        
                char ans[1024];
                memset(ans,0,1024);
                sprintf(ans,"GROUP %s CREATED\n",groupname);

                if((send(client_id, ans, strlen(ans), 0)) == -1){
                        printf("send_error");
                }
                return; 
            }


            else if(strcmp(first_word,"MCST")==0){
                second_word=strtok_r(NULL,sep,&brkt);
                char *message;
                message=strtok_r(NULL,sep,&brkt);
                char ans[1024];
                memset(ans,0,1024);
                for(int l=0;l<=group_counter;l++){
                    if(strcmp(second_word,groups[l].groupname)==0){
                        sprintf(ans,"%s:%s",vector[id],message);
                        for(int j=0;j<groups[l].total_users;j++){
                            int temp=groups[l].users[j];
                            if(ex[temp]==1)continue;
                            if((send(client_sockets[temp], ans, strlen(ans), 0)) == -1)printf("error in msct");
                        }
                        return;
                    }
                }
                sprintf(ans,"GROUP %s NOT FOUND\n",second_word);
                if((send(client_id, ans, strlen(ans), 0)) == -1)printf("error in msct");

            }
        }
        printf("INVALID CMD\n");
        // fprintf(file,"%s: INVALID CMD\n",vector[id]);
        return;
    }
    
}




// Function to handle client requests in a separate thread
void *client_handler(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int i = args->index;
    // fflush(file);
    // Find the id of the client in the array
    int client_id = client_connect(socket_id);
    client_sockets[i] = client_id;
    arrivals[i]=1;
    char info[1024];
    memset(info,0,0124);
    if (recv(client_id, info, 1024, 0) == -1) {
            perror("recv username");
            close(client_id);
            pthread_exit(NULL);
        }
    fprintf(file,"%s\n",info);
    fflush(file);

    ex[i]=0;
    char*brkt;
    char* username,*type,third_word;
    username=strtok_r(info,"|",&brkt);
    type=strtok_r(NULL,"|",&brkt);
    types[i]=0;
    if(strcmp(type,"r")==0){
        types[i]=1;
        char pass[1024];
        memset(pass,0,0124);
        if (recv(client_id, pass, 1024, 0) == -1) {
            perror("recv username");
            close(client_id);
            pthread_exit(NULL);
        }
        fprintf(file,"%s::%s\n",password,pass);
        fflush(file);
        if(strcmp(pass,password)==0){
            char* ans="PASSWORD ACCEPTED\n";
            if((send(client_id, ans, strlen(ans), 0)) == -1)printf("error in pssaloc");
        }
        else{
            char* ans="PASSWORD REJECTED\n";
            if((send(client_id, ans, strlen(ans), 0)) == -1)printf("error in pssaloc");
            close(client_id);
            ex[i]=1;
            pthread_exit(0);
        }
    }
    // fprintf(file,"%d:%s\n",i,username);
    // fflush(file);
    strcpy(vector[i],username);
        // Your existing code for initializing connections goes here...

    while (1 && ex[i]==0) {
        char command[1024];
        memset(command, 0, 1024);
        int temp;
        // Receive command from client
        if (recv(client_id, command, 1024, 0) == -1) {
            perror("recv");
            close(client_id);
            pthread_exit(NULL);
        }
        // fprintf(file,"%d:%s",counter,command);
        // fflush(file);fprintf(file,"%s",info);
    fflush(file);
        // Lock the mutex before accessing and updating the counter
        pthread_mutex_lock(&counter_mutex);

        // Fchar password[1024];ormat the string and store it in history[counter]   
        
        sprintf(history[counter], "%s-%s", vector[i], command);
        temp=counter;
        counter++;

        // Unlock the mutex after finishing the critical section
        pthread_mutex_unlock(&counter_mutex);

        // Handle command
        // Your existing solver function logic goes here...
        // Make sure to update the function signature accordingly

        // For demonstration, let's just echo back the received message
        solver(i,command,temp);
        if(ex[i]==1)pthread_exit(0);
    }
}


// Remaining functions remain unchanged

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Use 3 cli arguments\n");
        return -1;
    }
    n = MAX_CLIENTS;
    char addr[INET6_ADDRSTRLEN];
    
    memset(password,0,1024);
    strcpy(password,argv[3]);
    // file=fopen("log.txt","w");
    unsigned int port;
    strcpy(addr, argv[1]);
    port = atoi(argv[2]);
    socket_id = create_connection(addr, port);
    for (int i = 0; i < n; i++) {
        memset(vector[i], 0, sizeof(vector[i]));
    }
    
   

    for (int i = 0; i < n; i++) {
        memset(history[i], 0, sizeof(history[i]));
    }
    //First Connect
    file=fopen("log.txt","w");
    fprintf(file,"%s\n",password);
    fflush(file);
    // Create threads for each client connection
    counter=0;
    group_counter=-1;
    pthread_t threads[MAX_CLIENTS];
    ThreadArgs thread_args[MAX_CLIENTS];
    // fprintf(file,"here\n");
    // fflush(file);
    for(int i=0;i<n;i++)ex[i]=1;
    for (int i = 0; i < n; i++) {
        thread_args[i].index = i;
        arrivals[i]=0;
        while(!(i==0 || arrivals[i-1]==1)){
        }
        pthread_create(&threads[i], NULL, client_handler, (void *)&thread_args[i]);

    }

    // Wait for all threads to finish
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup and exit
    printf("SERVER TERMINATED: EXITING......\n");
   

    close(socket_id);
    return 0;
}
