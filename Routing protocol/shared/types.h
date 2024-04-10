#ifndef TYPES
#define TYPES
#define MAX_DV_ENTRIES 256

// Each entry in the payload(distance_vector) of the packet
struct dv_entry {
    int target_router; //8 bits
    int cost; //32 bits
};

// The routing packet being sent
struct packet {
    int source_router; //8 bits
    int dest_router; //8 bits
    int num_entries; //8 bits
    struct dv_entry distance_vector[MAX_DV_ENTRIES]; //Payload
};

// The local distance table for each router
struct distance_table
{
    int **costs;
};

#endif