#include "header.h"

int main(int argc, char *argv[])
{
    struct event *eventptr;

    FILE *file = fopen(argv[2], "r");
    char ch;
    num_nodes = 0;
    int max_iterations = atoi(argv[1]);
    while ((ch = fgetc(file)) != '\n') {
        if (ch == ' ') {
            num_nodes++;
        }
    }
    num_nodes++;
    dts = (struct distance_table *) malloc(num_nodes * sizeof(struct distance_table));
    link_costs = (int **) malloc(num_nodes * sizeof(int *));
    fseek(file, 0, SEEK_SET);
    for (int i = 0; i < num_nodes; i++)
    {
        link_costs[i] = (int *)malloc(num_nodes * sizeof(int));
        for(int j = 0; j < num_nodes; j++){
            fscanf(file, "%d", &link_costs[i][j]);
        }
    }    
    for (int i = 0; i < num_nodes; i++)
    {
        dts[i].costs = (int**) malloc(num_nodes*sizeof(int*));
        for(int j=0;j<num_nodes;j++){
            dts[i].costs[j] = (int *)malloc(num_nodes * sizeof(int));
            for(int k=0;k<num_nodes;k++){
                if(j==k)
                    dts[i].costs[j][k] = 0;
                else
                    dts[i].costs[j][k] = -1;
            }
        }
        router_init(&dts[i], i, link_costs[i], num_nodes);
    }
    int k = 0;  
    while (1) 
    {
        // printevlist();
        eventptr = evlist;
        if (eventptr==NULL || k==max_iterations)
            break;
        evlist = evlist->next;
        if (evlist!=NULL)
            evlist->prev=NULL;
        clocktime = eventptr->evtime;
        if(k<clocktime){
            printf("k=%d:\n",k);
            printdts();
            k++;
        }
        router_update(&dts[eventptr->eventity], eventptr->eventity, eventptr->packet_buffer_ptr, link_costs[eventptr->eventity], num_nodes);
        free(eventptr->packet_buffer_ptr);
        free(eventptr);
    }

    while(k<=max_iterations){
        printf("k=%d:\n",k);
        printdts();
        k++;
    }
    return 0;
}
