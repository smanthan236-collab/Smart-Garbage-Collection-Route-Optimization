#ifndef SMART_GARBAGE_ROUTING_H
#define SMART_GARBAGE_ROUTING_H
#include "dsa_types.h"
#include "graph.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Greedy nearest-neighbor visit order; each travel leg uses Dijkstra. */
int nearest_neighbor_route(const Graph *graph, int depot_node,
                           const CBin *bins, int bins_count,
                           const int *bin_ids, int bin_count,
                           CRouteResult *result);
#ifdef __cplusplus
}
#endif
#endif
