#include "dsa.h"

#include <math.h>
#include <stdlib.h>

#define CAPACITY_EPSILON 1e-9

int prioritize_bins(const CBin *bins, int count, int *ordered_bin_ids, int max_output)
{
    CBin *sorted;
    int output_count;
    if (bins == NULL || ordered_bin_ids == NULL || count <= 0 || max_output <= 0) return 0;

    sorted = (CBin *)malloc(sizeof(*sorted) * (size_t)count);
    if (sorted == NULL) return 0;
    for (int i = 0; i < count; ++i) {
        sorted[i] = bins[i];
        sorted[i].priority = calculate_bin_priority(bins[i].fill_level);
    }
    merge_sort_bins(sorted, count);
    output_count = count < max_output ? count : max_output;
    for (int i = 0; i < output_count; ++i) ordered_bin_ids[i] = sorted[i].bin_id;
    free(sorted);
    return output_count;
}

/*
 * Greedy best-fit assignment. Input bins must be in highest-priority-first order.
 * Each bin is sent to the feasible truck with the least capacity left after
 * taking the bin. A bin that fits no truck remains unassigned.
 */
int assign_bins_to_trucks(const CBin *bins, int bin_count,
                          const CTruck *trucks, int truck_count,
                          CTruckAssignment *assignments)
{
    int assigned_count = 0;
    if (bins == NULL || trucks == NULL || assignments == NULL ||
        bin_count <= 0 || truck_count <= 0) return 0;

    for (int t = 0; t < truck_count; ++t) {
        assignments[t].truck_id = trucks[t].truck_id;
        assignments[t].bin_count = 0;
        assignments[t].load_kg = 0.0;
    }

    for (int b = 0; b < bin_count; ++b) {
        int best_truck = -1;
        double best_remaining_after = INFINITY;
        const CBin *bin = &bins[b];
        if (!isfinite(bin->waste_kg) || bin->waste_kg < 0.0) continue;

        for (int t = 0; t < truck_count; ++t) {
            const double available = trucks[t].capacity_kg - trucks[t].current_load_kg
                                     - assignments[t].load_kg;
            const double remaining_after = available - bin->waste_kg;
            if (isfinite(available) && bin->waste_kg <= available + CAPACITY_EPSILON &&
                assignments[t].bin_count < MAX_BINS_PER_TRUCK &&
                remaining_after < best_remaining_after) {
                best_remaining_after = remaining_after;
                best_truck = t;
            }
        }

        if (best_truck >= 0) {
            CTruckAssignment *assignment = &assignments[best_truck];
            assignment->bin_ids[assignment->bin_count++] = bin->bin_id;
            assignment->load_kg += bin->waste_kg;
            ++assigned_count;
        }
    }
    return assigned_count;
}
