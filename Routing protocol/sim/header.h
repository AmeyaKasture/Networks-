#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <impl.h>

struct event {
    int evtime;
    int eventity;
    unsigned char* packet_buffer_ptr;
    struct event *prev;
    struct event *next;
};


struct event *evlist = NULL;
struct distance_table *dts;
int **link_costs;
int num_nodes;
int clocktime = 0;

void insertevent(struct event *p)
{
    struct event *q,*qold;

    q = evlist;
    if (q==NULL) {
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
    }
    else {
        for (qold = q; q !=NULL && p->evtime >= q->evtime; q=q->next)
            qold=q; 
        if (q==NULL) {
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q==evlist) {
            p->next=evlist;
            p->prev=NULL;
            p->next->prev=p;
            evlist = p;
        }
        else {
            p->next=q;
            p->prev=q->prev;
            q->prev->next=p;
            q->prev=p;
        }
    }
}

// void printevlist()
// {
//   struct event *q;
//   printf("--------------\nEvent List Follows:\n");
//   for(q = evlist; q!=NULL; q=q->next) {
//     printf("Event time: %d, entity: %d\n",q->evtime,q->eventity);
//     }
//   printf("--------------\n");
// }

void printdts()
{
    for(int node_num=0; node_num<num_nodes; node_num++){
        printf("node-%d.my_dt[%d]:",node_num, node_num);
        for(int other_node=0;other_node<num_nodes;other_node++){
            printf(" %d",dts[node_num].costs[node_num][other_node]);
        }
        printf("\n");
    }
}

void send2neighbor(unsigned char* buffer)
{
    struct event *evptr;
    struct packet* packet = deserialize(buffer);

    if (packet->source_router < 0 || packet->source_router > num_nodes) {
        printf("WARNING: illegal source id (%d) in your packet, ignoring packet!\n", packet->source_router);
        exit(1);
        return;
    }
    if (packet->dest_router < 0 || packet->dest_router > num_nodes) {
        printf("WARNING: illegal dest id (%d) in your packet, ignoring packet!\n", packet->dest_router);
        exit(1);
        return;
    }
    if (packet->source_router == packet->dest_router)  {
        printf("WARNING: source and destination id's the same (%d), ignoring packet!\n", packet->dest_router);
        exit(1);
        return;
    }
    if (link_costs[packet->source_router][packet->dest_router] == -1)  {
        printf("WARNING: no path between source and destination (%d -> %d), ignoring packet!\n", packet->source_router, packet->dest_router);
        exit(1);
        return;
    }

    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->eventity = packet->dest_router;
    evptr->packet_buffer_ptr = buffer;
    evptr->evtime = clocktime + 1;
    insertevent(evptr);
    free(packet);
} 

#endif
