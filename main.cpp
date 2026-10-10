#include "CSVManager.h"
#include "CollectionManager.h"
#include "DatabaseManager.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <string>

namespace {
namespace fs = std::filesystem;

bool importSeedData(DatabaseManager& database) {
    const bool binsImported = CSVManager::importBins("data/bins.csv", database);
    const bool trucksImported = CSVManager::importTrucks("data/trucks.csv", database);
    const bool roadsImported = CSVManager::importRoads("data/roads.csv", database);

    if (!binsImported || !trucksImported || !roadsImported) {
        std::cerr << "CSV import failed. Check the files in the data/ folder.\n";
        return false;
    }
    return true;
}

void showMenu() {
    std::cout << "\n==============================================\n"
              << " SMART GARBAGE COLLECTION & ROUTE OPTIMIZATION\n"
              << "==============================================\n"
              << "1. Display bins\n"
              << "2. Display trucks\n"
              << "3. Show bin priority order\n"
              << "4. Assign bins to trucks\n"
              << "5. Generate optimized routes\n"
              << "6. Update a bin's fill level\n"
              << "7. Collect currently assigned bins\n"
              << "8. Reload data from SQLite\n"
              << "9. Re-import CSV seed data\n"
              << "10. Exit\n"
              << "Select an option: ";
}

int readInteger(const std::string& prompt) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) return value;

        std::cout << "Please enter a whole number.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

double readDouble(const std::string& prompt) {
    double value = 0.0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) return value;

        std::cout << "Please enter a numeric value.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}
} // namespace

int main() {
    // Run the application from the project root so data/ and database/ resolve.
    std::error_code filesystemError;
    fs::create_directories("database", filesystemError);
    if (filesystemError) {
        std::cerr << "Could not create the database directory: "
                  << filesystemError.message() << '\n';
        return 1;
    }

    DatabaseManager database("database/garbage.db");
    if (!database.isOpen() || !database.createTables()) {
        std::cerr << "Database initialization failed.\n";
        return 1;
    }

    if (!database.hasBins() || !database.hasTrucks() || !database.hasRoads()) {
        std::cout << "Database is missing seed data; importing CSV files...\n";
        if (!importSeedData(database)) return 1;
    }

    CollectionManager collectionManager(database);
    int choice = 0;
    do {
        showMenu();
        if (!(std::cin >> choice)) {
            std::cout << "Please enter a menu number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
        case 1:
            collectionManager.displayBins();
            break;
        case 2:
            collectionManager.displayTrucks();
            break;
        case 3:
            collectionManager.displayPriorities();
            break;
        case 4:
            collectionManager.assignBins();
            collectionManager.displayTrucks();
            break;
        case 5:
            if (collectionManager.optimizeRoutes()) {
                collectionManager.displayRoutes();
            } else {
                std::cout << "No complete routes were generated. Assign bins and "
                             "check that all required road nodes are connected.\n";
            }
            break;
        case 6: {
            const int binId = readInteger("Bin ID: ");
            const double fill = readDouble("New fill level (0-100): ");
            if (fill < 0.0 || fill > 100.0) {
                std::cout << "Fill level must be between 0 and 100.\n";
            } else if (collectionManager.updateBin(binId, fill)) {
                std::cout << "Bin updated. Assignments were cleared; assign bins again.\n";
            } else {
                std::cout << "Could not update the bin. Check the bin ID.\n";
            }
            break;
        }
        case 7:
            if (collectionManager.collectAssigned()) {
                std::cout << "Assigned bins were recorded as collected.\n";
            } else {
                std::cout << "No assigned bins were collected. Assign bins first.\n";
            }
            break;
        case 8:
            collectionManager.reload();
            std::cout << "Reloaded bins, trucks, and roads from SQLite.\n";
            break;
        case 9:
            if (importSeedData(database)) {
                collectionManager.reload();
                std::cout << "CSV seed data imported. Existing matching IDs were updated.\n";
            }
            break;
        case 10:
            std::cout << "Exiting. Goodbye.\n";
            break;
        default:
            std::cout << "Choose an option from 1 to 10.\n";
            break;
        }
    } while (choice != 10);

    return 0;
}
