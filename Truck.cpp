#include "Truck.h"

#include <algorithm>

Truck::Truck(int id, double capacityKg)
    : id_(id), capacityKg_(std::max(0.0, capacityKg)), currentLoadKg_(0.0) {}

int Truck::getId() const { return id_; }
double Truck::getCapacityKg() const { return capacityKg_; }
double Truck::getCurrentLoadKg() const { return currentLoadKg_; }
double Truck::getRemainingCapacity() const {
    return std::max(0.0, capacityKg_ - currentLoadKg_);
}
const std::vector<int>& Truck::getAssignedBins() const { return assignedBins_; }

void Truck::clearAssignment() {
    assignedBins_.clear();
    currentLoadKg_ = 0.0;
}

bool Truck::canCollect(double wasteKg) const {
    return wasteKg >= 0.0 && wasteKg <= getRemainingCapacity() + 1e-9;
}

bool Truck::assignBin(int binId, double wasteKg) {
    if (!canCollect(wasteKg)) {
        return false;
    }

    assignedBins_.push_back(binId);
    currentLoadKg_ += wasteKg;
    return true
}
