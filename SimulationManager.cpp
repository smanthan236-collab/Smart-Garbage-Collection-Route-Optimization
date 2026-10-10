#include "SimulationManager.h"

#include <iostream>

SimulationManager::SimulationManager(CollectionManager& collectionManager)
    : collectionManager_(collectionManager) {
}

bool SimulationManager::runSimulation(bool collectAfterRouting) {
    std::cout << "\n==============================================\n"
              << "          STARTING SIMULATION CYCLE\n"
              << "==============================================\n";

    std::cout << "\n[Step 1/4] Current bin status\n";
    collectionManager_.displayBins();

    std::cout << "\n[Step 2/4] Bin priority order\n";
    collectionManager_.displayPriorities();

    std::cout << "\n[Step 3/4] Assigning bins to trucks\n";
    if (!collectionManager_.assignBins()) {
        std::cout << "Simulation stopped: no bins could be assigned.\n";
        return false;
    }
    collectionManager_.displayTrucks();

    std::cout << "\n[Step 4/4] Optimizing collection routes\n";
    if (!collectionManager_.optimizeRoutes()) {
        std::cout << "Simulation could not create all required routes. "
                     "Check the road network and assigned bins.\n";
        return false;
    }
    collectionManager_.displayRoutes();

    if (collectAfterRouting) {
        std::cout << "\nRecording completed collections...\n";
        if (!collectionManager_.collectAssigned()) {
            std::cout << "Warning: no assigned bins were recorded as collected.\n";
            return false;
        }

        std::cout << "Collection records saved. Updated bin status:\n";
        collectionManager_.displayBins();
    } else {
        std::cout << "\nSimulation preview complete. No bins have been marked "
                     "as collected.\n";
    }

    std::cout << "==============================================\n"
              << "             SIMULATION COMPLETE\n"
              << "==============================================\n";

    return true;
}
