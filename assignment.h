#ifndef SMART_GARBAGE_ASSIGNMENT_H
#define SMART_GARBAGE_ASSIGNMENT_H
#include "dsa_types.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Returns assigned bin count; bins should be sorted highest-priority first. */
int assign_bins_to_trucks(const CBin *bins, int bin_count,
                          const CTruck *trucks, int truck_count,
                          CTruckAssignment *assignments);
int prioritize_bins(const CBin *bins, int count, int *ordered_bin_ids, int max_output);
#ifdef __cplusplus
}
#endif
#endif
