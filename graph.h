#ifndef SMART_GARBAGE_GRAPH_H
#define SMART_GARBAGE_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Graph Graph;

Graph *graph_create(int node_count);
void graph_free(Graph *graph);
int graph_add_edge(Graph *graph, int source, int destination, double distance);
int graph_node_count(const Graph *graph);
/* Dijkstra all-destinations API: dist and parent each need node_count elements. */
int graph_dijkstra(const Graph *graph, int source, double *dist, int *parent);
/* Returns path node count, or 0 if no path/error. */
int graph_shortest_path(const Graph *graph, int source, int destination,
                        double *out_distance, int *out_path, int max_path_nodes);

#ifdef __cplusplus
}
#endif
#endif
