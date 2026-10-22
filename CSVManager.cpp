#include "CSVManager.h"

#include "DatabaseManager.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

bool skipHeader(std::ifstream& input) {
    std::string header;
    return static_cast<bool>(std::getline(input, header));
}

bool parseInteger(const std::string& text, int& value) {
    try {
        std::size_t used = 0;
        value = std::stoi(text, &used);
        return used == text.size();
    } catch (...) {
        return false;
    }
}

bool parseDouble(const std::string& text, double& value) {
    try {
        std::size_t used = 0;
        value = std::stod(text, &used);
        return used == text.size();
    } catch (...) {
        return false;
    }
}
} // namespace

bool CSVManager::importBins(const std::string& filename, DatabaseManager& db) {
    std::ifstream input(filename);
    if (!input || !skipHeader(input)) {
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;

        const auto fields = splitCsvLine(line);
        int id = 0;
        int nodeId = 0;
        double fill = 0.0;
        double capacity = 0.0;
        if (fields.size() < 5 || !parseInteger(fields[0], id) ||
            !parseInteger(fields[1], nodeId) || !parseDouble(fields[3], fill) ||
            !parseDouble(fields[4], capacity) || fill < 0.0 || fill > 100.0 ||
            capacity < 0.0) {
            return false;
        }

        if (!db.insertBin(Bin(id, nodeId, fields[2], fill, capacity))) {
            return false;
        }
    }
    return !input.bad();
}

bool CSVManager::importTrucks(const std::string& filename, DatabaseManager& db) {
    std::ifstream input(filename);
    if (!input || !skipHeader(input)) {
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;

        const auto fields = splitCsvLine(line);
        int id = 0;
        double capacity = 0.0;
        if (fields.size() < 2 || !parseInteger(fields[0], id) ||
            !parseDouble(fields[1], capacity) || capacity < 0.0) {
            return false;
        }
        if (!db.insertTruck(Truck(id, capacity))) {
            return false;
        }
    }
    return !input.bad();
}

bool CSVManager::importRoads(const std::string& filename, DatabaseManager& db) {
    std::ifstream input(filename);
    if (!input || !skipHeader(input)) {
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;

        const auto fields = splitCsvLine(line);
        int source = 0;
        int destination = 0;
        double distance = 0.0;
        if (fields.size() < 3 || !parseInteger(fields[0], source) ||
            !parseInteger(fields[1], destination) ||
            !parseDouble(fields[2], distance) || source < 0 ||
            destination < 0 || distance <= 0.0) {
            return false;
        }
        if (!db.insertRoad({source, destination, distance})) {
            return false;
        }
    }
    return !input.bad();
}
