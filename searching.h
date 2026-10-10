#ifndef SMART_GARBAGE_SEARCHING_H
#define SMART_GARBAGE_SEARCHING_H
#include "dsa_types.h"
#ifdef __cplusplus
extern "C" {
#endif
int linear_search_bin_id(const CBin *bins, int count, int bin_id);
/* Binary search requires bins sorted by bin_id ascending. */
int binary_search_bin_id(const CBin *bins, int count, int bin_id);
#ifdef __cplusplus
}
#endif
#endif
