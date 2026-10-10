#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H

#include "CollectionManager.h"

/**
 * Coordinates one complete garbage-collection simulation cycle.
 *
 * CollectionManager remains responsible for the actual bin, truck,
 * database, priority-queue, and routing operations. SimulationManager
 * organizes those operations into a single, easy-to-call workflow.
 */
class SimulationManager {
public:
    explicit SimulationManager(CollectionManager& collectionManager);

    /**
     * Runs one simulation cycle:
     *   1. Display current bin priorities.
     *   2. Assign eligible bins to trucks.
     *   3. Generate and display routes.
     *   4. Optionally record the assigned bins as collected.
     *
     * Returns true when assignment and route generation succeed.
     * Set collectAfterRouting to true to record the collection in SQLite.
     */
    bool runSimulation(bool collectAfterRouting = false);

private:
    CollectionManager& collectionManager_;
};

#endif // SIMULATION_MANAGER_H
