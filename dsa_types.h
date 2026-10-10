#ifndef SMART_GARBAGE_DSA_TYPES_H
#define SMART_GARBAGE_DSA_TYPES_H

/* Shared plain-C data structures. Keep these free of C++-only types. */
#define MAX_BINS_PER_TRUCK 1024
#define MAX_ROUTE_NODES 1024

typedef struct {
    int bin_id;
    int node_id;
    double fill_level;
    double capacity_kg;
    double priority;
    double waste_kg;
} CBin;

typedef struct {
    int truck_id;
    double capacity_kg;
    double current_load_kg;
} CTruck;

typedef struct {
    int truck_id;
    int bin_count;
    int bin_ids[MAX_BINS_PER_TRUCK];
    double load_kg; /* load assigned by this planning run */
} CTruckAssignment;

typedef struct {
    int truck_id;
    int node_count;
    int nodes[MAX_ROUTE_NODES];
    double distance;
    double load_kg;
} CRouteResult;

#endif
