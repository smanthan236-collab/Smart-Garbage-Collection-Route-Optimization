#ifndef ROUTE_H
#define ROUTE_H

#include <vector>

class Route {
public:
    explicit Route(int truckId = 0);

    void setNodes(const std::vector<int>& nodes);
    void setDistance(double distanceKm);
    void setLoad(double loadKg);

    int getTruckId() cons;
    const std::vector<int>& getNodes() const;
    double getDistance() const;
    double getLoad() const;

private:
    int truckId_;
    std::vector<int> nodes_;
    double distanceKm_;
    double loadKg_;
};

#endif // ROUTE_H
