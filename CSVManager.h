#ifndef CSV_MANAGER_H
#define CSV_MANAGER_H

#include <string>

#include "Bin.h"
#include "Truck.h"

struct Road {
    int source;
    int destination;
    double distance;
};

class DatabaseManager;

// CSV is used for initial seed data; SQLite stores the imported record.
class CSVManager {
public:
    static bool importBins(const std::string& filename, DatabaseManager& db);
    static bool importTrucks(const std::string& filename, DatabaseManager& db);
    static bool importRoads(const std::string& filename, DatabaseManager& db);
};

#endif // CSV_MANAGER_H
