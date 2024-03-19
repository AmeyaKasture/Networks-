#include "header.h"

void printPacket(struct packet *p){  //always follow this format for printing the packet
    printf("{\n");
    printf("    Version: %d\n", p->version);
    printf("    Header Length: %d\n", p->headerLength);
    printf("    Total Length: %d\n", p->totalLength);
    printf("    Source Department: %d\n", p->srcDept);
    printf("    Destination Department: %d\n", p->destDept);
    printf("    CheckSum: %d\n", p->checkSum);
    printf("    Hops: %d\n", p->hops);
    printf("    Type: %d\n", p->type);
    printf("    ACK: %d\n", p->ACK);
    if(p->headerLength == 6){
        printf("    Source Campus: %d\n", p->srcCampus);
        printf("    Destination Campus: %d\n", p->destCampus);
    }
    printf("    Data: %s\n", p->data);
    printf("}\n");
}

unsigned char* serialize( struct packet* p){
    unsigned char *buffer = (unsigned char *)malloc(1032 * sizeof(unsigned char));
    memset(buffer,0,1032);
    uint8_t version= p->version;
    uint8_t headerLength=p->headerLength;
    uint8_t totalLength=p->totalLength;
    uint8_t srcDept=p->srcDept;
    uint8_t destDept=p->destDept;
    uint16_t checkSum=p->checkSum;
    uint8_t hops=p->hops;
    uint8_t type=p->type;
    uint8_t ACK=p->ACK;
    uint8_t srcCampus=p->srcCampus;
    uint8_t destCampus=p->destCampus;
    uint8_t mask = 0x00;
    uint8_t masky = 0xFF;
    buffer[0] =  mask | (version << 4) | headerLength;
    buffer[1]=totalLength;
    buffer[2]=  mask| (srcDept<< 5) | (destDept<< 2) | (uint8_t)(checkSum >>8);
    buffer[3]= mask| (uint8_t)(checkSum &  masky);
    buffer[4]= mask| (hops<< 5) | (type<< 2) | (ACK);
    if(headerLength==6){
        buffer[5]= mask| (srcCampus << 4) | destCampus;
        strcpy(buffer+6,p->data);
    }
    else strcpy(buffer+5,p->data);
    
    //code for serialization
    return buffer;
}

struct packet *deserialize(unsigned char* buffer){
   struct packet *p = malloc(sizeof(struct packet));
    if (p == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Deserialization logic
    p->version = (buffer[0] >> 4) & 0xF;
    p->headerLength = buffer[0] & 0xF;
    p->totalLength = buffer[1];
    p->srcDept = (buffer[2] >> 5) & 0x7;
    p->destDept = (buffer[2] >> 2) & 0x7;
    p->checkSum = ((buffer[2] & 0x3) << 8) | (buffer[3] & 0xFF);
    p->hops = (buffer[4] >> 5) & 0x7;
    p->type = (buffer[4] >> 2) & 0x7;
    p->ACK = buffer[4] & 0x3;
    if (p->headerLength == 6) {
        p->srcCampus = (buffer[5] >> 4) & 0xF;
        p->destCampus = buffer[5] & 0xF;
        strcpy(p->data, buffer + 6); // Assuming data starts from index 6
    }
    else strcpy(p->data, buffer + 5);
    return p;
}

struct packet *generatePacket(int version, int headerLength, int totalLength, 
                              int srcDept, int destDept, int checkSum, int hops, 
                              int type, int ACK, int srcCampus, int destCampus, char* data) {
    //feel free to write your own function with more customisations. This is a very basic func 
    struct packet *p;
    p = (struct packet *)malloc(sizeof(struct packet));
    p->version = version;
    p->headerLength = headerLength;
    p->totalLength = totalLength;
    p->srcDept = srcDept;
    p->destDept = destDept;
    p->checkSum = checkSum;
    p->hops = hops;
    p->type = type;
    p->ACK = ACK;
    p->srcCampus = srcCampus;
    p->destCampus = destCampus;
    strcpy(p->data, data);
    return p;
}
