#include "impl.h"
#include <stdio.h>




// void printer(FILE* u,struct distance_table *my_dt,int num_nodes){
//     for(int i=0;i<num_nodes;i++){
//         for(int j=0;j<num_nodes;j++){
//             fprintf(u,"%d ",my_dt->costs[i][j]);
//         }
//         fprintf(u,"\n");
//     }
//     fflush(u);
// }



unsigned char* serialize(struct packet* pkt) {
    unsigned char *buffer = (unsigned char *)malloc(1032 * sizeof(unsigned char));
    memset(buffer,0,1032);
    // FILE * s=fopen("serialise.txt","a+");
    buffer[0]=pkt->source_router & 0xFF;
    buffer[1]=pkt->dest_router & 0xFF;
    buffer[2]=pkt->num_entries & 0xFF;
    int i=0;
    int j=3;
    // fprintf(s,"source router is%d\n",pkt->source_router);
    while(i<pkt->num_entries){
        int target_router=pkt->distance_vector[i].target_router;
        int cost=pkt->distance_vector[i].cost;
        buffer[j]=target_router ;
        j++;
        buffer[j]=cost>>24;
        j++;
        buffer[j]=cost>>16 & 0xFF;
        j++;
        buffer[j]=cost>>8 & 0xFF;
        j++;
        buffer[j]=cost & 0xFF;
        j++;
        i++;
        // fprintf(s,"\ttarget router=%d\n",target_router);
        // fflush(s);
    }
    // fprintf(s,"%d   buffered to->>> %d",pkt->dest_router,buffer[1]);
    // fflush(s);
    //code for serialization
    return buffer;
}
struct packet *deserialize(unsigned char* buffer) {
    struct packet* pkt = (struct packet*)malloc(sizeof(struct packet));
    pkt->source_router=buffer[0];
    pkt->dest_router=buffer[1];
    pkt->num_entries=buffer[2];
    int i=0;
    int j=3;
    while(i<pkt->num_entries){
        pkt->distance_vector[i].target_router=buffer[j];
        j++;
        int cost=0;
        cost+=buffer[j]<<24;
        j++;
        cost+=buffer[j]<<16;
        j++;
        cost+=buffer[j]<<8;
        j++;
        cost+=buffer[j];
        j++;
        pkt->distance_vector[i].cost=cost;
        i++;
    }
    return pkt;
}

void router_init(struct distance_table *my_dt, int my_node, int *my_link_costs, int num_nodes)
{
    for(int i=0;i<num_nodes;i++){
        my_dt->costs[my_node][i]=my_link_costs[i];
    }
    struct packet* pkt = (struct packet*)malloc(sizeof(struct packet));
    pkt->num_entries=0;
    for(int i=0;i<num_nodes;i++){
        if(i!=my_node && my_link_costs[i]!=-1){
            pkt->distance_vector[pkt->num_entries].target_router=i;
            pkt->distance_vector[pkt->num_entries].cost=my_link_costs[i];
            pkt->num_entries++;
        }
    }
    // FILE* f=fopen("router_init.txt","a+");
    // fprintf(f,"router init of %d\n",my_node);
    // fflush(f);
    for(int i=0;i<num_nodes;i++){
        if(i!=my_node && my_link_costs[i]!=-1){
            // fprintf(f,"router %d:\n",my_node);
            // fflush(f);
            pkt->source_router=my_node;
            pkt->dest_router=i;
            // fprintf(f,"\t%d :: %d\n",pkt->source_router,pkt->dest_router);
            // fflush(f);
            send2neighbor(serialize(pkt));
        }
    }
    // fprintf(f,"router init completed of %d",my_node);
    // fflush(f);
}

void router_update(struct distance_table *my_dt, int my_node, unsigned char* packet_buffer, int *my_link_costs, int num_nodes)
{
    int changer=0;
    int flag[num_nodes];
    int init[num_nodes];
    for(int i=0;i<num_nodes;i++)init[i]=my_dt->costs[my_node][i];
    for(int i=0;i<num_nodes;i++){
        int direct=my_dt->costs[my_node][i];
        int indirect=my_link_costs[i];
        if(direct==-1 || indirect<direct)my_dt->costs[my_node][i]=indirect;
        if(my_dt->costs[my_node][i]!=init[i]){
            flag[i]=1;
            changer=1;}
    }
    // FILE * u=fopen("update_route.txt","a+");
    // fprintf(u,"updating router:%d\n",my_node);
    struct packet* p=(struct packet* )malloc(sizeof(struct packet));
    p=deserialize(packet_buffer);
    for(int i=0;i<p->num_entries;i++){
        // fprintf(u,"src=%d,target=%d,cost=%d,number=%d\n",p->source_router,p->distance_vector[i].target_router,p->distance_vector[i].cost,num_nodes);
        // fflush(u);
        my_dt->costs[p->source_router][p->distance_vector[i].target_router]=p->distance_vector[i].cost;
    }
    // printer(u,my_dt,num_nodes);
    for(int i=0;i<num_nodes;i++){
         for(int j=0;j<num_nodes;j++){
            if(my_dt->costs[my_node][j]!=-1 && my_dt->costs[j][i]!=-1 ){
                int direct=my_dt->costs[my_node][i];
                int indirect=my_dt->costs[my_node][j]+my_dt->costs[j][i];
                // fprintf(u,"node:%d:\n\tdirect:%d\n\tindirect from %d:%d\n",i,direct,j,indirect);
                // fflush(u);
                if(direct==-1 || indirect<direct)my_dt->costs[my_node][i]=indirect;
                // my_dt->costs[my_node][i]=min(direct,indirect);
            }
            if(my_dt->costs[my_node][i]!=init[i]){
            flag[i]=1;
            changer=1;}
        }
    }
    // fprintf(u,"the changer is %d\n",changer);
    // fflush(u);
    if(changer==1){
        // printer(u,my_dt,num_nodes);
        
        struct packet* pkt = (struct packet*)malloc(sizeof(struct packet));
        pkt->num_entries=0;
        for(int i=0;i<num_nodes;i++){
            if(flag[i]==1){
                pkt->distance_vector[pkt->num_entries].target_router=i;
                pkt->distance_vector[pkt->num_entries].cost=my_dt->costs[my_node][i];
                pkt->num_entries++;
            }
        }
        for(int i=0;i<num_nodes;i++){
            if(i!=my_node && my_link_costs[i]!=-1){
                pkt->source_router=my_node;
                pkt->dest_router=i;
                send2neighbor(serialize(pkt));
            }
        }
    }
    // fprintf(u,"\n");
    // fflush(u);
}
