#ifndef COLLECTION_MANAGER_H
#define COLLECTION_MANAGER_H

#include <vector>

#include "Bin.h"
#include "DatabaseManager.h"
#include "PriorityManager.h"
#include "Route.h"
#include "Truck.h"
#include "../C_DSA/dsa.h"

// Coordinates the C++ domain objects with the C data-structure algorithms.
class CollectionManager {
public:
    explicit CollectionManager(DatabaseManager& database);
    ~CollectionManager();

    CollectionManager(const CollectionManager&) = delete;
    CollectionManager& operator=(const CollectionManager&) = delete;

    void reload();
    void displayBins() const;
    void displayTrucks() const;
    void displayPriorities() const;

    bool assignBins();
    bool optimizeRoutes();
    bool updateBin(int binId, double fillLevel);
    void displayRoutes() const;
    bool collectAssigned();

    const std::vector<Bin>& bins() const;
    const std::vector<Truck>& trucks() const;

private:
    DatabaseManager& database_;
    std::vector<Bin> bins_;
    std::vector<Truck> trucks_;
    std::vector<Route> routes_;
    Graph* graph_;

    bool buildGraph();
};

#endif // COLLECTION_MANAGER_H
