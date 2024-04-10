#include "header.h"

#define PILANI 0
#define GOA 1
#define HYDERABAD 2

int pilaniSocketId, goaSocketId, hydSocketId;
int campus;
int arrivals[100];
int socket_id;
typedef struct {
    int index;
} ThreadArgs;

typedef struct{
    int srcCampus;
    int srcCampus_port;
}receive_args;


typedef struct{
    int id;
    int sock_id;
    char name[1024];
    int exit_state;
}Department;

Department departments[3][100];

int pc=0,gc=0,hc=0;
int numOfValidDept[3];
int validDept[3][7];

void *recv_thread(void *arg) {
    receive_args *args = (receive_args *)arg;
    int srcCampus=args->srcCampus;
    int srcCampus_port=args->srcCampus_port;
    char message[1024];
    while (1) {
        memset(message,0,1024);
        int recv_count = recv(srcCampus_port, message, 1024, 0);
        if (recv_count == -1) {
            perror("recv");
            exit(1);
        } else if (recv_count == 0) {
            printf("Connection closed by server\n");
            exit(0);
        } else {
            char data[1024];
            memset(data,0,1024);
            struct packet* p=deserialize(message);
            strcpy(data,p->data);
            if(p->headerLength==6){
                char* saveptr,*word;
                char value[1024];
                for(word=strtok_r(data,"|",&saveptr);word;word=strtok_r(NULL,"|",&saveptr)){
                    memset(value,0,1024);
                    strcpy(value,word);
                    char* word2,*saveptr2;
                    word2=strtok_r(value,";",&saveptr2);
                    int department_id;
                    if(word2[0]=='P')department_id=0;
                    else if(word2[0]=='G')department_id=1;
                    else department_id=2;
                    int count=0;
                    for(word2=strtok_r(NULL,";",&saveptr2);word2;word2=strtok_r(NULL,";",&saveptr2)){
                        char dept[1024];
                        memset(dept,0,1024);
                        strcpy(dept,word2);
                        char* saveptr3;
                        char * name,*id;
                        name=strtok_r(dept,"=",&saveptr3);
                        id=strtok_r(NULL,"=",&saveptr3);
                        validDept[department_id][count]=atoi(id);
                        
                    }
                }


            }
            
        }
    }
    return NULL;

}



int client_create_connection(char* addr,int port){
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
    if(listen(server_sockfd, 5) == -1) {
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




int serverSetupPilani(char* addr, int goaPort, int hydPort){
    campus = PILANI;
    int goa_server = create_connection(addr,goaPort);
    int goa_port=client_connect(goa_server);
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    char message[1024];
    char reply[1024];
    //initialising with goa campus

    memset(reply,0,1024);
    memset(message,0,1024);
    p = generatePacket(2,6,6,0,0,0,0,0,2,1,2,"");
    strcpy(message,serialize(p));
    if((send(goa_port,message, strlen(message), 0)) == -1)printf("error in sending to goa by pilani\n");
    free(p);
    if (recv(goa_port,reply, 1024, 0) == -1) {
            perror("recv username");
            close(goa_port);
            pthread_exit(NULL);
    }
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p= deserialize(reply);
    printf("%d\n",p->ACK);
    free(p);
    receive_args rcg;
    rcg.srcCampus= GOA;
    rcg.srcCampus_port=goa_port;
    pthread_t tg;
    pthread_create(&tg, NULL, recv_thread, (void *)&rcg);




    // initialising with the hyderabad campus

    int hyd_server = create_connection(addr,hydPort);
    int hyd_port=client_connect(hyd_server);
    memset(reply,0,1024);
    memset(message,0,1024);
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p = generatePacket(2,6,6,0,0,0,0,0,2,1,3,"");
    strcpy(message,serialize(p));
    if((send(hyd_port,message, strlen(message), 0)) == -1)printf("error in sending to goa by pilani\n");
    free(p);
    if (recv(hyd_port,reply, 1024, 0) == -1) {
            perror("recv from hyd to pilani");
            close(hyd_port);
    }
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p= deserialize(reply);
    printf("%d\n",p->ACK);
    free(p);
    receive_args rch;
    rch.srcCampus= HYDERABAD;
    rch.srcCampus_port=hyd_port;
    pthread_t th;
    pthread_create(&th, NULL, recv_thread, (void *)&rch);
    
    pthread_join(tg,NULL);
    printf("goa thread of pilani campus joined\n");
    pthread_join(th,NULL);
    printf("hyderabad thread of pilani campus joined\n");

    return 1;
}

int serverSetupGoa(char* addr, int pilaniPort, int hydPort){
    campus = GOA;
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    char message[1024];
    char reply[1024];
    //connecting with pilani port 
    int pilani_port = client_create_connection(addr,pilaniPort);
    memset(reply,0,1024);
    memset(message,0,1024);
    if(recv(pilani_port,reply, 1024, 0) == -1) {
            perror("recv pilani-goa");
            close(pilani_port);
    } 
    p=deserialize(reply);
    printf("%d\n",p->ACK);
    free(p);

    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p = generatePacket(2,6,6,0,0,0,0,0,1,2,1,"");
    strcpy(message,serialize(p));
    if((send(pilani_port,message, strlen(message), 0)) == -1)printf("error in sending by goa to pilani\n");
    free(p);

    receive_args rcp;
    rcp.srcCampus= PILANI;
    rcp.srcCampus_port=pilani_port;
    pthread_t tp;
    pthread_create(&tp, NULL, recv_thread, (void *)&rcp);


    //connecting with hyd port
    int hyd_server = create_connection(addr,hydPort);
    int hyd_port=client_connect(hyd_server);
    memset(reply,0,1024);
    memset(message,0,1024);
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p = generatePacket(2,6,6,0,0,0,0,0,2,2,3,"");
    strcpy(message,serialize(p));
    if((send(hyd_port,message, strlen(message), 0)) == -1)printf("error in sending to hyd by goa\n");
    free(p);
    if (recv(hyd_port,reply, 1024, 0) == -1) {
            perror("recv from hyd to goa");
            close(hyd_port);
    }
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p= deserialize(reply);
    printf("%d\n",p->ACK);
    free(p);

    receive_args rch;
    rch.srcCampus= HYDERABAD;
    rch.srcCampus_port=hyd_port;
    pthread_t th;
    pthread_create(&th, NULL, recv_thread, (void *)&rch);


    pthread_join(tp,NULL);
    printf("pilani thread of campus joined\n");
    pthread_join(th,NULL);
    printf("hyderabad thread of goa campus\n");

    return 1;
    
}

int serverSetupHyderabad(char* addr, int pilaniPort, int goaPort){
    campus = HYDERABAD;
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    char message[1024];
    char reply[1024];
    //connecting to pilani campus
    int pilani_port=client_create_connection(addr,pilaniPort);
    memset(reply,0,1024);
    memset(message,0,1024);
    if(recv(pilani_port,reply, 1024, 0) == -1) {
            perror("recv pilani-hyd");
            close(pilani_port);
    } 
    p=deserialize(reply);
    printf("%d\n",p->ACK);
    free(p);


    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p = generatePacket(2,6,6,0,0,0,0,0,1,3,1,"");
    strcpy(message,serialize(p));
    if((send(pilani_port,message, strlen(message), 0)) == -1)printf("error in sending by goa to pilani\n");
    free(p);

    receive_args rcp;
    rcp.srcCampus= PILANI;
    rcp.srcCampus_port=pilani_port;
    pthread_t tp;
    pthread_create(&tp, NULL, recv_thread, (void *)&rcp);

    //conneting to goa campus
    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet));
    int goa_port = client_create_connection(addr,goaPort);
    memset(reply,0,1024);
    memset(message,0,1024);
    if(recv(goa_port,reply, 1024, 0) == -1) {
            perror("recv goa-hyd");
            close(goa_port);
    }
    p=deserialize(reply);
    printf("%d\n",p->ACK);
    free(p);


    struct packet*p;
    p = (struct packet *)malloc(sizeof(struct packet)); 
    
    p = generatePacket(2,6,6,0,0,0,0,0,1,3,1,"");
    strcpy(message,serialize(p));
    if((send(goa_port,message, strlen(message), 0)) == -1)printf("error in sending by goa to pilani\n");
    free(p);

    receive_args rcg;
    rcg.srcCampus= GOA;
    rcg.srcCampus_port=goa_port;
    pthread_t tg;
    pthread_create(&tg, NULL, recv_thread, (void *)&rcg);

    pthread_join(tp,NULL);
    printf("pilani thread of hyd campus joined\n");
    pthread_join(tg,NULL);
    printf("goa thread of hyd campus\n");


    return 1;
   
}




void * client_handler(void* arg){
    ThreadArgs *args = (ThreadArgs *)arg;
    int ind = args->index;
    int client_id = client_connect(socket_id);
    int dest;
    for(int i=1;i<6;i++){
        if(departments[campus][i].exit_state==1){
            departments[campus][i].sock_id = client_id;
            departments[campus][i].id=i;
            departments[campus][i].exit_state=0;
            validDept[campus][numOfValidDept[campus]]=i;
            numOfValidDept[campus]++;
            dest=i;
        }
    }
    arrivals[ind]=1;
    char buffer[1024];
    memset(buffer,0,1024);
    char name[1024];
    memset(name,0,1024);
    if(recv(client_id, buffer, 1024, 0) == -1) {
            perror("recv username");
            close(client_id);
            pthread_exit(NULL);
    }
    struct packet *p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p=deserialize(buffer);
    
    char reply[1024];
    memset(reply,0,1024);
    strcpy(reply,generateAcknowledgmentPacket(0,dest,validDept,numOfValidDept));
    if((send(client_id, reply, strlen(reply), 0)) == -1){
            printf("send_error");
            exit(1);
    }





}

int main(int argc, char *argv[])
{
    if (argc != 5)
	{
		printf("Refer Qn for arguments\n");
		return -1;
	}
    char* addr = argv[1];
	int port1 = atoi(argv[2]);
    int port2 = atoi(argv[3]);
    int port3 = atoi(argv[4]);
    campus = atoi(argv[5]);
    
    if(campus == PILANI){
        serverSetupPilani(addr, port1, port2);
    }
    else if(campus == GOA){
        serverSetupGoa(addr, port1, port2);
    }
    else if(campus == HYDERABAD){
        serverSetupHyderabad(addr, port1, port2);
    }
    else{
        printf("Invalid campus\n");
        return -1;
    }
    for(int i=0;i<7;i++){
        memset(departments[campus][i].name,0,1024);
        
        departments[campus][i].exit_state=1;
    }
    int socket_id = create_connection(addr, port3);
    pthread_t threads[100];
    ThreadArgs thread_args[100];
    for (int i = 0; i < 100; i++) {
        thread_args[i].index = i;
        arrivals[i]=0;
        while(!(i==0 || arrivals[i-1]==1)){
        }
        pthread_create(&threads[i], NULL, client_handler, (void *)&thread_args[i]);
    }
    for (int i = 0; i < 100; i++) {
        pthread_join(threads[i], NULL);
    }


  
}



