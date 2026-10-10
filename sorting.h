#ifndef SMART_GARBAGE_SORTING_H
#define SMART_GARBAGE_SORTING_H
#include "dsa_types.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Sort descending by priority, ascending bin ID for ties. */
void merge_sort_bins(CBin *bins, int count);
#ifdef __cplusplus
}
#endif
#endif
