include "PriorityManager.h"

PriorityManager::PriorityManager(int initialCapacity)
    : queue_(pq_create(initialCapacity > 0 ? initialCapacity : 1)) {}

PriorityManager::~PriorityManager() { pq_free(queue_); }

bool PriorityManager::addBin(int binId, double priority) {
    return queue_ != nullptr && pq_push(queue_, binId, priority) != 0;
}

bool PriorityManager::popHighestPriority(PriorityItem& item) {
    return queue_ != nullptr &&
           pq_pop(queue_, &item.binId, &item.priority) != 0;
}

bool PriorityManager::isEmpty() const {
    return queue_ == nullptr || pq_empty(queue_) != 0;
}

bool PriorityManager::clear(int initialCapacity) {
    pq_free(queue_);
    queue_ = pq_create(initialCapacity > 0 ? initialCapacity : 1);
    return queue_ != nullptr;
}

std::vector<PriorityItem> PriorityManager::drainInPriorityOrder() {
    std::vector<PriorityItem> items;
    PriorityItem item{};
    while (popHighestPriority(item)) {
        items.push_back(item);
    }
    return items;
}
