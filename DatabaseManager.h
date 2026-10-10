#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>

#include "Bin.h"
#include "CSVManager.h"
#include "Truck.h"

struct sqlite3;

// Owns the SQLite connection and all persistence operations.
class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& filename);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool isOpen() const;
    void close();
    bool execute(const std::string& sql);
    bool createTables();

    bool insertBin(const Bin& bin);
    bool insertTruck(const Truck& truck);
    bool insertRoad(const Road& road);

    std::vector<Bin> loadBins() const;
    std::vector<Truck> loadTrucks() const;
    std::vector<Road> loadRoads() const;

    bool updateBinFill(int binId, double fillLevel);
    bool markBinCollected(int binId);
    bool saveRoute(int truckId, const std::vector<int>& nodes,
                   double distanceKm, double loadKg);
    bool recordCollection(int truckId, int binId, double amountKg);

    bool hasBins() const;
    bool hasTrucks() const;
    bool hasRoads() const;

private:
    sqlite3* db_;
};

#endif // DATABASE_MANAGER_H
