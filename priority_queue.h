#ifndef SMART_GARBAGE_PRIORITY_QUEUE_H
#define SMART_GARBAGE_PRIORITY_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PriorityQueue PriorityQueue;

double calculate_bin_priority(double fill_level);
PriorityQueue *pq_create(int initial_capacity);
void pq_free(PriorityQueue *queue);
int pq_push(PriorityQueue *queue, int bin_id, double priority);
int pq_pop(PriorityQueue *queue, int *bin_id, double *priority);
int pq_empty(const PriorityQueue *queue);

#ifdef __cplusplus
}
#endif
#endif
