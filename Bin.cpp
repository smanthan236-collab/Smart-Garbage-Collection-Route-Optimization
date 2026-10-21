#include "Bin.h"

#include "../C_DSA/dsa.h"

#include <algorithm>
#include <utility>

Bin::Bin(int id, int nodeId, std::string location, double fillLevel,
         double capacityKg)
    : id_(id), nodeId_(nodeId), location_(std::move(location)),
      fillLevel_(0.0), capacityKg_(std::max(0.0, capacityKg)),
      collected_(false) {
    setFillLevel(fillLevel);
}


int Bin::getId() const { return id_; }
int Bin::getNodeId() const { return nodeId_; }
const std::string& Bin::getLocation() const { return location_; }
double Bin::getFillLevel() const { return fillLevel_; }
double Bin::getCapacityKg() const { return capacityKg_; }
bool Bin::isCollected() const { return collected_; }

double Bin::getWasteKg() const {
    return capacityKg_ * fillLevel_ / 100.0;
}

double Bin::getPriority() const {
    return calculate_bin_priority(fillLevel_);
}

void Bin::setFillLevel(double fillLevel) {
    fillLevel_ = std::clamp(fillLevel, 0.0, 100.0);
}

void Bin::setCollected(bool collected) { collected_ = collected; }
//binid
