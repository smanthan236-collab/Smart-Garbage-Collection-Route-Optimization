#include "priority_queue.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

/* The scoring policy is intentionally simple and explainable in the PBL. */
double calculate_bin_priority(double fill_level)
{
    if (!isfinite(fill_level)) return 0.0;
    if (fill_level < 0.0) fill_level = 0.0;
    if (fill_level > 100.0) fill_level = 100.0;

    if (fill_level >= 90.0) return 100.0 + fill_level / 100.0;
    if (fill_level >= 75.0) return 80.0 + fill_level / 100.0;
    if (fill_level >= 50.0) return 60.0 + fill_level / 100.0;
    if (fill_level >= 25.0) return 40.0 + fill_level / 100.0;
    return 20.0 + fill_level / 100.0;
}

typedef struct {
    int bin_id;
    double priority;
} PQItem;

struct PriorityQueue {
    PQItem *items;
    int size;
    int capacity;
};

PriorityQueue *pq_create(int initial_capacity)
{
    PriorityQueue *queue;
    if (initial_capacity < 1) initial_capacity = 1;
    queue = (PriorityQueue *)malloc(sizeof(*queue));
    if (queue == NULL) return NULL;
    queue->items = (PQItem *)malloc(sizeof(PQItem) * (size_t)initial_capacity);
    if (queue->items == NULL) {
        free(queue);
        return NULL;
    }
    queue->size = 0;
    queue->capacity = initial_capacity;
    return queue;
}

void pq_free(PriorityQueue *queue)
{
    if (queue == NULL) return;
    free(queue->items);
    free(queue);
}

static void swap_items(PQItem *left, PQItem *right)
{
    PQItem temp = *left;
    *left = *right;
    *right = temp;
}

int pq_push(PriorityQueue *queue, int bin_id, double priority)
{
    int index;
    if (queue == NULL || !isfinite(priority)) return 0;

    if (queue->size == queue->capacity) {
        int new_capacity;
        PQItem *expanded;
        if (queue->capacity > 2147483647 / 2) return 0;
        new_capacity = queue->capacity * 2;
        expanded = (PQItem *)realloc(queue->items,
                                     sizeof(PQItem) * (size_t)new_capacity);
        if (expanded == NULL) return 0;
        queue->items = expanded;
        queue->capacity = new_capacity;
    }

    index = queue->size++;
    queue->items[index].bin_id = bin_id;
    queue->items[index].priority = priority;
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (queue->items[parent].priority >= queue->items[index].priority) break;
        swap_items(&queue->items[parent], &queue->items[index]);
        index = parent;
    }
    return 1;
}

int pq_pop(PriorityQueue *queue, int *bin_id, double *priority)
{
    PQItem top;
    int index = 0;
    if (queue == NULL || queue->size == 0) return 0;

    top = queue->items[0];
    --queue->size;
    if (queue->size > 0) {
        queue->items[0] = queue->items[queue->size];
        for (;;) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;
            if (left < queue->size &&
                queue->items[left].priority > queue->items[largest].priority) largest = left;
            if (right < queue->size &&
                queue->items[right].priority > queue->items[largest].priority) largest = right;
            if (largest == index) break;
            swap_items(&queue->items[index], &queue->items[largest]);
            index = largest;
        }
    }
    if (bin_id != NULL) *bin_id = top.bin_id;
    if (priority != NULL) *priority = top.priority;
    return 1;
}

int pq_empty(const PriorityQueue *queue)
{
    return queue == NULL || queue->size == 0;
}
