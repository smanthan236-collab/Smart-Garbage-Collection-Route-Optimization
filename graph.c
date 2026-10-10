#include "graph.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct Edge {
    int to;
    double weight;
    struct Edge *next;
} Edge;

struct Graph {
    int node_count;
    Edge **adjacency;
};

Graph *graph_create(int node_count)
{
    Graph *graph;
    if (node_count <= 0) return NULL;
    graph = (Graph *)malloc(sizeof(*graph));
    if (graph == NULL) return NULL;
    graph->adjacency = (Edge **)calloc((size_t)node_count, sizeof(Edge *));
    if (graph->adjacency == NULL) {
        free(graph);
        return NULL;
    }
    graph->node_count = node_count;
    return graph;
}

void graph_free(Graph *graph)
{
    if (graph == NULL) return;
    for (int node = 0; node < graph->node_count; ++node) {
        Edge *edge = graph->adjacency[node];
        while (edge != NULL) {
            Edge *next = edge->next;
            free(edge);
            edge = next;
        }
    }
    free(graph->adjacency);
    free(graph);
}

static Edge *create_edge(int to, double weight, Edge *next)
{
    Edge *edge = (Edge *)malloc(sizeof(*edge));
    if (edge == NULL) return NULL;
    edge->to = to;
    edge->weight = weight;
    edge->next = next;
    return edge;
}

int graph_add_edge(Graph *graph, int source, int destination, double distance)
{
    Edge *forward;
    Edge *reverse;
    if (graph == NULL || source < 0 || destination < 0 ||
        source >= graph->node_count || destination >= graph->node_count ||
        source == destination || !isfinite(distance) || distance <= 0.0) return 0;

    /* Allocate both halves before modifying the graph, avoiding half-added roads. */
    forward = create_edge(destination, distance, graph->adjacency[source]);
    if (forward == NULL) return 0;
    reverse = create_edge(source, distance, graph->adjacency[destination]);
    if (reverse == NULL) {
        free(forward);
        return 0;
    }
    graph->adjacency[source] = forward;
    graph->adjacency[destination] = reverse;
    return 1;
}

int graph_node_count(const Graph *graph)
{
    return graph == NULL ? 0 : graph->node_count;
}

int graph_dijkstra(const Graph *graph, int source, double *dist, int *parent)
{
    unsigned char *visited;
    if (graph == NULL || dist == NULL || parent == NULL ||
        source < 0 || source >= graph->node_count) return 0;
    visited = (unsigned char *)calloc((size_t)graph->node_count, sizeof(*visited));
    if (visited == NULL) return 0;

    for (int i = 0; i < graph->node_count; ++i) {
        dist[i] = DBL_MAX;
        parent[i] = -1;
    }
    dist[source] = 0.0;

    for (int step = 0; step < graph->node_count; ++step) {
        int current = -1;
        double best = DBL_MAX;
        for (int i = 0; i < graph->node_count; ++i) {
            if (!visited[i] && dist[i] < best) {
                best = dist[i];
                current = i;
            }
        }
        if (current < 0) break;
        visited[current] = 1;
        for (Edge *edge = graph->adjacency[current]; edge != NULL; edge = edge->next) {
            if (!visited[edge->to] && dist[current] != DBL_MAX &&
                dist[current] + edge->weight < dist[edge->to]) {
                dist[edge->to] = dist[current] + edge->weight;
                parent[edge->to] = current;
            }
        }
    }
    free(visited);
    return 1;
}

int graph_shortest_path(const Graph *graph, int source, int destination,
                        double *out_distance, int *out_path, int max_path_nodes)
{
    double *dist;
    int *parent;
    int *reverse_path;
    int count = 0;
    int result = 0;
    if (graph == NULL || out_distance == NULL || out_path == NULL || max_path_nodes <= 0 ||
        source < 0 || destination < 0 || source >= graph->node_count ||
        destination >= graph->node_count) return 0;

    dist = (double *)malloc(sizeof(double) * (size_t)graph->node_count);
    parent = (int *)malloc(sizeof(int) * (size_t)graph->node_count);
    reverse_path = (int *)malloc(sizeof(int) * (size_t)graph->node_count);
    if (dist == NULL || parent == NULL || reverse_path == NULL) goto cleanup;
    if (!graph_dijkstra(graph, source, dist, parent) || dist[destination] == DBL_MAX) goto cleanup;

    for (int node = destination; node != -1; node = parent[node]) {
        if (count >= graph->node_count) goto cleanup;
        reverse_path[count++] = node;
        if (node == source) break;
    }
    if (count == 0 || reverse_path[count - 1] != source || count > max_path_nodes) goto cleanup;
    for (int i = 0; i < count; ++i) out_path[i] = reverse_path[count - i - 1];
    *out_distance = dist[destination];
    result = count;

cleanup:
    free(dist);
    free(parent);
    free(reverse_path);
    return result;
}
