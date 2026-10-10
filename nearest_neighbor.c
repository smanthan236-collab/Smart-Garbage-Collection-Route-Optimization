#include "routing.h"
#include "searching.h"

#include <float.h>
#include <stddef.h>

/* The C++ database loader supplies bins ordered by ascending bin_id. */
static int find_bin_index(const CBin *bins, int count, int bin_id)
{
    return binary_search_bin_id(bins, count, bin_id);
}

/*
 * Greedy nearest-neighbor heuristic:
 * 1. From the current node, find the nearest unvisited assigned bin.
 * 2. Use Dijkstra to calculate the shortest road path to that bin.
 * 3. Repeat until every assigned bin is visited, then return to the depot.
 *
 * This is a practical heuristic, not a guarantee of the globally shortest tour.
 * The bins array must be sorted by bin_id because lookup uses binary search.
 */
int nearest_neighbor_route(const Graph *graph, int depot_node,
                           const CBin *bins, int bins_count,
                           const int *bin_ids, int bin_count,
                           CRouteResult *result)
{
    unsigned char visited[MAX_ROUTE_NODES] = {0};
    int current_node = depot_node;

    if (graph == NULL || bins == NULL || bin_ids == NULL || result == NULL ||
        bins_count <= 0 || bin_count < 0 || bin_count > MAX_ROUTE_NODES - 2) {
        return 0;
    }

    result->node_count = 0;
    result->distance = 0.0;
    result->nodes[result->node_count++] = depot_node;

    for (int stop = 0; stop < bin_count; ++stop) {
        double best_distance = DBL_MAX;
        int best_assigned_index = -1;
        int best_path_length = 0;
        int best_path[MAX_ROUTE_NODES];

        for (int assigned_index = 0; assigned_index < bin_count; ++assigned_index) {
            double candidate_distance = 0.0;
            int candidate_path[MAX_ROUTE_NODES];
            int candidate_path_length;
            int bin_index;

            if (visited[assigned_index]) {
                continue;
            }

            bin_index = find_bin_index(bins, bins_count, bin_ids[assigned_index]);
            if (bin_index < 0) {
                continue;
            }

            candidate_path_length = graph_shortest_path(
                graph, current_node, bins[bin_index].node_id,
                &candidate_distance, candidate_path, MAX_ROUTE_NODES);

            if (candidate_path_length > 0 && candidate_distance < best_distance) {
                best_distance = candidate_distance;
                best_assigned_index = assigned_index;
                best_path_length = candidate_path_length;
                for (int path_index = 0; path_index < candidate_path_length; ++path_index) {
                    best_path[path_index] = candidate_path[path_index];
                }
            }
        }

        if (best_assigned_index < 0) {
            return 0; /* At least one assigned bin is missing or unreachable. */
        }

        visited[best_assigned_index] = 1;
        for (int path_index = 1; path_index < best_path_length; ++path_index) {
            if (result->node_count >= MAX_ROUTE_NODES) {
                return 0;
            }
            result->nodes[result->node_count++] = best_path[path_index];
        }

        result->distance += best_distance;
        current_node = best_path[best_path_length - 1];
    }

    if (current_node != depot_node) {
        double return_distance = 0.0;
        int return_path[MAX_ROUTE_NODES];
        const int return_path_length = graph_shortest_path(
            graph, current_node, depot_node, &return_distance,
            return_path, MAX_ROUTE_NODES);

        if (return_path_length <= 0) {
            return 0;
        }

        for (int path_index = 1; path_index < return_path_length; ++path_index) {
            if (result->node_count >= MAX_ROUTE_NODES) {
                return 0;
            }
            result->nodes[result->node_count++] = return_path[path_index];
        }
        result->distance += return_distance;
    }

    return result->node_count;
}
