#include "Route.h"

#include <algorithm>

Route::Route(int truckId)
    : truckId_(truckId), distanceKm_(0.0), loadKg_(0.0) {}

void Route::setNodes(const std::vector<int>& nodes) { nodes_ = nodes; }
void Route::setDistance(double distanceKm) {
    distanceKm_ = std::max(0.0, distanceKm);
}
void Route::setLoad(double loadKg) { loadKg_ = std::max(0.0, loadKg); }

int Route::getTruckId() const { return truckId_; }
const std::vector<int>& Route::getNodes() const { return nodes_; }
double Route::getDistance() const { return distanceKm_; }
double Route::getLoad() const { return loadKg_; }
