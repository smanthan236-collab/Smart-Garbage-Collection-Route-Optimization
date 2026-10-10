#ifndef TRUCK_H
#define TRUCK_H

#include <vector>

class Truck {
public:
    Truck(int id = 0, double capacityKg = 0.0);

    int getId() const;
    double getCapacityKg() const;
    double getCurrentLoadKg() const;
    double getRemainingCapacity() const;
    const std::vector<int>& getAssignedBins() const;

    void clearAssignment();
    bool canCollect(double wasteKg) const;
    bool assignBin(int binId, double wasteKg);

private:
    int id_;
    double capacityKg_;
    double currentLoadKg_;
    std::vector<int> assignedBins_;
};

#endif // TRUCK_H
