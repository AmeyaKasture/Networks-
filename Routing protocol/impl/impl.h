#ifndef IMPL_H
#define IMPL_H

#include <stdlib.h>
#include <string.h>
#include <types.h>

// To be implemented
unsigned char* serialize(struct packet* p);
struct packet* deserialize(unsigned char* buffer);
void router_init(struct distance_table *my_dt, int my_node, int *my_link_costs, int num_nodes);
void router_update(struct distance_table *my_dt, int my_node, unsigned char* packet_buffer, int *my_link_costs, int num_nodes);

// Provided to you
// You have to call the functions wherever appropriate
extern void send2neighbor(unsigned char* buffer);

#endif
