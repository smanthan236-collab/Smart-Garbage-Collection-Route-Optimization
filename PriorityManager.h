#ifndef PRIORITY_MANAGER_H
#define PRIORITY_MANAGER_H

#include <vector>
#include "../C_DSA/dsa.h"

struct PriorityItem {
    int binId;
    double priority;
};

// Readable C++ interface over the priority queue implemented in C.
class PriorityManager {
public:
    explicit PriorityManager(int initialCapacity = 16);
    ~PriorityManager();

    PriorityManager(const PriorityManager&) = delete;
    PriorityManager& operator=(const PriorityManager&) = delete;

    bool addBin(int binId, double priority);
    bool popHighestPriority(PriorityItem& item);
    bool isEmpty() const;
    bool clear(int initialCapacity = 16);
    std::vector<PriorityItem> drainInPriorityOrder();

private:
    PriorityQueue* queue_;
};

#endif // PRIORITY_MANAGER_H
