#include "searching.h"
#include <stddef.h>

int linear_search_bin_id(const CBin *bins, int count, int bin_id)
{
    if (bins == NULL || count < 0) {
        return -1;
    }

    for (int index = 0; index < count; ++index) {
        if (bins[index].bin_id == bin_id) {
            return index;
        }
    }

    return -1;
}

/* The input array must be sorted by bin_id in ascending order. */
int binary_search_bin_id(const CBin *bins, int count, int bin_id)
{
    int low = 0;
    int high;

    if (bins == NULL || count <= 0) {
        return -1;
    }

    high = count - 1;
    while (low <= high) {
        const int middle = low + (high - low) / 2;

        if (bins[middle].bin_id == bin_id) {
            return middle;
        }
        if (bins[middle].bin_id < bin_id) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }

    return -1;
}
