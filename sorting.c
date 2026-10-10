#include "sorting.h"

#include <stdlib.h>

/* Sort by descending priority; use ascending ID to break ties consistently. */
static int comes_before_or_equal(const CBin *left, const CBin *right)
{
    if (left->priority != right->priority) {
        return left->priority > right->priority;
    }
    return left->bin_id <= right->bin_id;
}

static void merge_ranges(CBin *bins, CBin *temporary,
                         int first, int middle, int last)
{
    int left = first;
    int right = middle + 1;
    int output = first;

    while (left <= middle && right <= last) {
        if (comes_before_or_equal(&bins[left], &bins[right])) {
            temporary[output++] = bins[left++];
        } else {
            temporary[output++] = bins[right++];
        }
    }
    while (left <= middle) {
        temporary[output++] = bins[left++];
    }
    while (right <= last) {
        temporary[output++] = bins[right++];
    }
    for (int index = first; index <= last; ++index) {
        bins[index] = temporary[index];
    }
}

static void merge_sort_range(CBin *bins, CBin *temporary, int first, int last)
{
    int middle;

    if (first >= last) {
        return;
    }

    middle = first + (last - first) / 2;
    merge_sort_range(bins, temporary, first, middle);
    merge_sort_range(bins, temporary, middle + 1, last);
    merge_ranges(bins, temporary, first, middle, last);
}

void merge_sort_bins(CBin *bins, int count)
{
    CBin *temporary;

    if (bins == NULL || count < 2) {
        return;
    }

    temporary = (CBin *)malloc((size_t)count * sizeof(*temporary));
    if (temporary == NULL) {
        return;
    }

    merge_sort_range(bins, temporary, 0, count - 1);
    free(temporary);
}
