#include "DatabaseManager.h"

#include <sqlite3.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

DatabaseManager::DatabaseManager(const std::string& filename) : db_(nullptr) {
    const int result = sqlite3_open(filename.c_str(), &db_);
    if (result != SQLITE_OK) {
        std::cerr << "Could not open SQLite database: "
                  << (db_ ? sqlite3_errmsg(db_) : "unknown error") << '\n';
        close();
    }
}

DatabaseManager::~DatabaseManager() { close(); }

bool DatabaseManager::isOpen() const { return db_ != nullptr; }

void DatabaseManager::close() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::execute(const std::string& sql) {
    if (!db_) return false;

    char* errorMessage = nullptr;
    const int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr,
                                    &errorMessage);
    if (result != SQLITE_OK) {
        std::cerr << "SQLite error: "
                  << (errorMessage ? errorMessage : sqlite3_errmsg(db_))
                  << '\n';
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

bool DatabaseManager::createTables() {
    return execute("PRAGMA foreign_keys = ON;") &&
           execute("CREATE TABLE IF NOT EXISTS bins ("
                   "bin_id INTEGER PRIMARY KEY, node_id INTEGER NOT NULL, "
                   "location TEXT NOT NULL, fill_level REAL NOT NULL "
                   "CHECK(fill_level >= 0 AND fill_level <= 100), "
                   "capacity_kg REAL NOT NULL CHECK(capacity_kg >= 0), "
                   "collected INTEGER NOT NULL DEFAULT 0);") &&
           execute("CREATE TABLE IF NOT EXISTS trucks ("
                   "truck_id INTEGER PRIMARY KEY, "
                   "max_capacity_kg REAL NOT NULL CHECK(max_capacity_kg >= 0));") &&
           execute("CREATE TABLE IF NOT EXISTS roads ("
                   "source_node INTEGER NOT NULL, destination_node INTEGER NOT NULL, "
                   "distance REAL NOT NULL CHECK(distance > 0), "
                   "PRIMARY KEY(source_node, destination_node));") &&
           execute("CREATE TABLE IF NOT EXISTS routes ("
                   "route_id INTEGER PRIMARY KEY AUTOINCREMENT, truck_id INTEGER, "
                   "route_text TEXT NOT NULL, total_distance REAL NOT NULL, "
                   "total_load REAL NOT NULL, created_at TEXT DEFAULT CURRENT_TIMESTAMP);") &&
           execute("CREATE TABLE IF NOT EXISTS collections ("
                   "collection_id INTEGER PRIMARY KEY AUTOINCREMENT, truck_id INTEGER, "
                   "bin_id INTEGER, amount_kg REAL NOT NULL, "
                   "created_at TEXT DEFAULT CURRENT_TIMESTAMP);");
}

bool DatabaseManager::insertBin(const Bin& bin) {
    const char* sql = "INSERT OR REPLACE INTO bins "
                      "(bin_id,node_id,location,fill_level,capacity_kg,collected) "
                      "VALUES(?,?,?,?,?,?);";
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(statement, 1, bin.getId());
    sqlite3_bind_int(statement, 2, bin.getNodeId());
    sqlite3_bind_text(statement, 3, bin.getLocation().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 4, bin.getFillLevel());
    sqlite3_bind_double(statement, 5, bin.getCapacityKg());
    sqlite3_bind_int(statement, 6, bin.isCollected() ? 1 : 0);
    const bool success = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return success;
}

bool DatabaseManager::insertTruck(const Truck& truck) {
    const char* sql = "INSERT OR REPLACE INTO trucks(truck_id,max_capacity_kg) "
                      "VALUES(?,?);";
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(statement, 1, truck.getId());
    sqlite3_bind_double(statement, 2, truck.getCapacityKg());
    const bool success = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return success;
}

bool DatabaseManager::insertRoad(const Road& road) {
    const char* sql = "INSERT OR REPLACE INTO roads "
                      "(source_node,destination_node,distance) VALUES(?,?,?);";
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(statement, 1, road.source);
    sqlite3_bind_int(statement, 2, road.destination);
    sqlite3_bind_double(statement, 3, road.distance);
    const bool success = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return success;
}

std::vector<Bin> DatabaseManager::loadBins() const {
    std::vector<Bin> bins;
    const char* sql = "SELECT bin_id,node_id,location,fill_level,capacity_kg,collected "
                      "FROM bins ORDER BY bin_id;";
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return bins;

    while (sqlite3_step(statement) == SQLITE_ROW) {
        const auto* location = sqlite3_column_text(statement, 2);
        Bin bin(sqlite3_column_int(statement, 0), sqlite3_column_int(statement, 1),
                location ? reinterpret_cast<const char*>(location) : "",
                sqlite3_column_double(statement, 3),
                sqlite3_column_double(statement, 4));
        bin.setCollected(sqlite3_column_int(statement, 5) != 0);
        bins.push_back(bin);
    }
    sqlite3_finalize(statement);
    return bins;
}

std::vector<Truck> DatabaseManager::loadTrucks() const {
    std::vector<Truck> trucks;
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_,
            "SELECT truck_id,max_capacity_kg FROM trucks ORDER BY truck_id;",
            -1, &statement, nullptr) != SQLITE_OK) return trucks;

    while (sqlite3_step(statement) == SQLITE_ROW) {
        trucks.emplace_back(sqlite3_column_int(statement, 0),
                            sqlite3_column_double(statement, 1));
    }
    sqlite3_finalize(statement);
    return trucks;
}

std::vector<Road> DatabaseManager::loadRoads() const {
    std::vector<Road> roads;
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_,
            "SELECT source_node,destination_node,distance FROM roads "
            "ORDER BY source_node,destination_node;", -1, &statement, nullptr) != SQLITE_OK)
        return roads;

    while (sqlite3_step(statement) == SQLITE_ROW) {
        roads.push_back({sqlite3_column_int(statement, 0),
                         sqlite3_column_int(statement, 1),
                         sqlite3_column_double(statement, 2)});
    }
    sqlite3_finalize(statement);
    return roads;
}

bool DatabaseManager::updateBinFill(int binId, double fillLevel) {
    const char* sql = "UPDATE bins SET fill_level=?, collected=0 WHERE bin_id=?;";
    sqlite3_stmt* statement = nullptr;
    if (!db_ || fillLevel < 0.0 || fillLevel > 100.0 ||
        sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_double(statement, 1, fillLevel);
    sqlite3_bind_int(statement, 2, binId);
    const bool success = sqlite3_step(statement) == SQLITE_DONE &&
                         sqlite3_changes(db_) > 0;
    sqlite3_finalize(statement);
    return success;
}

bool DatabaseManager::markBinCollected(int binId) {
    sqlite3_stmt* statement = nullptr;
    if (!db_ || sqlite3_prepare_v2(db_,
            "UPDATE bins SET fill_level=0,collected=1 WHERE bin_id=?;",
            -1, &statement, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(statement, 1, binId);
    const bool success = sqlite3_step(statement) == SQLITE_DONE &&
                         sqlite3_changes(db_) > 0;
    sqlite3_finalize(statement);
    return success;
}

bool DatabaseManager::saveRoute(int truckId, const std::vector<int>& nodes,
                                double distanceKm, double loadKg) {
    std::ostringstream routeText;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) routeText << " -> ";
        routeText << nodes[i];
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "INSERT INTO routes(truck_id,route_text,total_distance,total_load) "
                      "VALUES(?,?,?,?);";
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    const std::string route = routeText.str();
    sqlite3_bind_int(statement, 1, truckId);
    sqlite3_bind_text(statement, 2, route.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 3, distanceKm);
    sqlite3_bind_double(statement, 4, loadKg);
    const bool success = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return success;
}

bool DatabaseManager::recordCollection(int truckId, int binId, double amountKg) {
    sqlite3_stmt* statement = nullptr;
    const char* sql = "INSERT INTO collections(truck_id,bin_id,amount_kg) VALUES(?,?,?);";
    if (!db_ || sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(statement, 1, truckId);
    sqlite3_bind_int(statement, 2, binId);
    sqlite3_bind_double(statement, 3, amountKg);
    const bool success = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    return success;
}

bool DatabaseManager::hasBins() const { return !loadBins().empty(); }
bool DatabaseManager::hasTrucks() const { return !loadTrucks().empty(); }
bool DatabaseManager::hasRoads() const { return !loadRoads().empty(); }
