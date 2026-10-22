#include "CollectionManager.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

CollectionManager::CollectionManager(DatabaseManager& database)
    : database_(database), graph_(nullptr) {
    reload();
}

CollectionManager::~CollectionManager() { graph_free(graph_); }

void CollectionManager::reload() {
    bins_ = database_.loadBins();
    trucks_ = database_.loadTrucks();
    routes_.clear();

    for (auto& truck : trucks_) {
        truck.clearAssignment();
    }

    graph_free(graph_);
    graph_ = nullptr;
    buildGraph();
}

bool CollectionManager::buildGraph() {
    const std::vector<Road> roads = database_.loadRoads();
    int highestNodeId = 0;

    for (const Road& road : roads) {
        highestNodeId = std::max(highestNodeId,
                                 std::max(road.source, road.destination));
    }
    for (const Bin& bin : bins_) {
        highestNodeId = std::max(highestNodeId, bin.getNodeId());
    }

    graph_ = graph_create(highestNodeId + 1);
    if (!graph_) return false;

    for (const Road& road : roads) {
        if (!graph_add_edge(graph_, road.source, road.destination, road.distance)) {
            graph_free(graph_);
            graph_ = nullptr;
            return false;
        }
    }
    return true;
}

void CollectionManager::displayBins() const {
    std::cout << "\n" << std::left << std::setw(6) << "ID"
              << std::setw(7) << "Node" << std::setw(25) << "Location"
              << std::right << std::setw(9) << "Fill %"
              << std::setw(13) << "Waste kg"
              << std::setw(11) << "Priority" << '\n';
    std::cout << std::string(71, '-') << '\n';

    for (const Bin& bin : bins_) {
        std::cout << std::left << std::setw(6) << bin.getId()
                  << std::setw(7) << bin.getNodeId()
                  << std::setw(25) << bin.getLocation()
                  << std::right << std::fixed << std::setprecision(1)
                  << std::setw(9) << bin.getFillLevel()
                  << std::setw(13) << bin.getWasteKg()
                  << std::setw(11) << bin.getPriority();
        if (bin.isCollected()) std::cout << "  collected";
        std::cout << '\n';
    }
}

void CollectionManager::displayTrucks() const {
    std::cout << "\n" << std::left << std::setw(10) << "Truck ID"
              << std::right << std::setw(14) << "Capacity kg"
              << std::setw(12) << "Load kg"
              << std::setw(17) << "Remaining kg" << '\n';
    std::cout << std::string(53, '-') << '\n';

    for (const Truck& truck : trucks_) {
        std::cout << std::left << std::setw(10) << truck.getId()
                  << std::right << std::fixed << std::setprecision(1)
                  << std::setw(14) << truck.getCapacityKg()
                  << std::setw(12) << truck.getCurrentLoadKg()
                  << std::setw(17) << truck.getRemainingCapacity() << '\n';
    }
}

void CollectionManager::displayPriorities() const {
    if (bins_.empty()) {
        std::cout << "No bins are currently stored in the database.\n";
        return;
    }

    PriorityManager priorityQueue(static_cast<int>(bins_.size()));
    for (const Bin& bin : bins_) {
        if (!priorityQueue.addBin(bin.getId(), bin.getPriority())) {
            std::cout << "Could not add a bin to the priority queue.\n";
            return;
        }
    }

    std::cout << "\nBin priority order (highest first)\n";
    PriorityItem item{};
    while (priorityQueue.popHighestPriority(item)) {
        std::cout << "Bin " << item.binId << " | priority: "
                  << std::fixed << std::setprecision(2) << item.priority << '\n';
    }
}

bool CollectionManager::assignBins() {
    for (Truck& truck : trucks_) truck.clearAssignment();
    routes_.clear();

    if (bins_.empty() || trucks_.empty()) {
        std::cout << "Assignment needs at least one bin and one truck.\n";
        return false;
    }

    std::vector<CBin> candidates;
    std::unordered_map<int, CBin> binsById;
    candidates.reserve(bins_.size());

    for (const Bin& bin : bins_) {
        if (bin.isCollected() || bin.getFillLevel() <= 0.0) continue;
        const CBin candidate{bin.getId(), bin.getNodeId(), bin.getFillLevel(),
                             bin.getCapacityKg(), bin.getPriority(), bin.getWasteKg()};
        candidates.push_back(candidate);
        binsById.emplace(candidate.bin_id, candidate);
    }

    if (candidates.empty()) {
        std::cout << "There are no uncollected bins with waste to assign.\n";
        return false;
    }

    // Use the C priority queue through the C++ wrapper to order the bins.
    PriorityManager priorityQueue(static_cast<int>(candidates.size()));
    for (const CBin& bin : candidates) {
        if (!priorityQueue.addBin(bin.bin_id, bin.priority)) return false;
    }

    std::vector<CBin> priorityOrderedBins;
    PriorityItem item{};
    while (priorityQueue.popHighestPriority(item)) {
        const auto found = binsById.find(item.binId);
        if (found != binsById.end()) priorityOrderedBins.push_back(found->second);
    }

    std::vector<CTruck> cTrucks;
    cTrucks.reserve(trucks_.size());
    for (const Truck& truck : trucks_) {
        cTrucks.push_back({truck.getId(), truck.getCapacityKg(), 0.0});
    }

    std::vector<CTruckAssignment> assignments(trucks_.size());
    const int assignedCount = assign_bins_to_trucks(
        priorityOrderedBins.data(), static_cast<int>(priorityOrderedBins.size()),
        cTrucks.data(), static_cast<int>(cTrucks.size()), assignments.data());

    for (std::size_t truckIndex = 0; truckIndex < trucks_.size(); ++truckIndex) {
        for (int i = 0; i < assignments[truckIndex].bin_count; ++i) {
            const int binId = assignments[truckIndex].bin_ids[i];
            const auto found = binsById.find(binId);
            if (found != binsById.end()) {
                trucks_[truckIndex].assignBin(binId, found->second.waste_kg);
            }
        }
    }

    std::cout << "Assigned " << assignedCount << " bin(s) across "
              << trucks_.size() << " truck(s).\n";
    return assignedCount > 0;
}

bool CollectionManager::optimizeRoutes() {
    if (!graph_) {
        std::cout << "Road graph could not be created.\n";
        return false;
    }

    routes_.clear();
    std::vector<CBin> cBins;
    cBins.reserve(bins_.size());
    for (const Bin& bin : bins_) {
        cBins.push_back({bin.getId(), bin.getNodeId(), bin.getFillLevel(),
                         bin.getCapacityKg(), bin.getPriority(), bin.getWasteKg()});
    }

    bool allRoutesCreated = true;
    for (const Truck& truck : trucks_) {
        const auto& assignedBins = truck.getAssignedBins();
        if (assignedBins.empty()) continue;

        CRouteResult result{};
        result.truck_id = truck.getId();
        const int nodeCount = nearest_neighbor_route(
            graph_, 0, cBins.data(), static_cast<int>(cBins.size()),
            assignedBins.data(), static_cast<int>(assignedBins.size()), &result);

        if (nodeCount <= 0) {
            std::cout << "Could not find a complete route for truck "
                      << truck.getId() << ". Check road connectivity.\n";
            allRoutesCreated = false;
            continue;
        }

        Route route(truck.getId());
        route.setNodes(std::vector<int>(result.nodes, result.nodes + result.node_count));
        route.setDistance(result.distance);
        route.setLoad(truck.getCurrentLoadKg());
        if (!database_.saveRoute(route.getTruckId(), route.getNodes(),
                                 route.getDistance(), route.getLoad())) {
            std::cout << "Warning: route could not be saved to SQLite.\n";
        }
        routes_.push_back(route);
    }

    return allRoutesCreated && !routes_.empty();
}

void CollectionManager::displayRoutes() const {
    if (routes_.empty()) {
        std::cout << "No routes generated yet. Assign bins first.\n";
        return;
    }

    std::cout << "\nOptimized truck routes (nearest-neighbor + Dijkstra)\n";
    for (const Route& route : routes_) {
        std::cout << "Truck " << route.getTruckId() << " | load: "
                  << std::fixed << std::setprecision(1) << route.getLoad()
                  << " kg | distance: " << route.getDistance() << " km\n  ";
        const auto& nodes = route.getNodes();
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << nodes[i];
        }
        std::cout << '\n';
    }
}

bool CollectionManager::updateBin(int binId, double fillLevel) {
    if (fillLevel < 0.0 || fillLevel > 100.0) return false;
    const bool updated = database_.updateBinFill(binId, fillLevel);
    if (updated) reload();
    return updated;
}

bool CollectionManager::collectAssigned() {
    bool collectedAny = false;
    for (const Truck& truck : trucks_) {
        for (int binId : truck.getAssignedBins()) {
            const auto found = std::find_if(bins_.begin(), bins_.end(),
                [binId](const Bin& bin) { return bin.getId() == binId; });
            if (found == bins_.end()) continue;

            if (database_.recordCollection(truck.getId(), binId, found->getWasteKg()) &&
                database_.markBinCollected(binId)) {
                collectedAny = true;
            }
        }
    }
    r();
    return collectedAny;
}

const std::vector<Bin>& CollectionManager::bins() const { return bins_; }
const std::vector<Truck>& CollectionManager::trucks() const { return trucks_; }
