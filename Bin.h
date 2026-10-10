#ifndef BIN_H
#define BIN_H

#include <string>

class Bin {
public:
    Bin(int id = 0, int nodeId = 0, std::string location = "",
        double fillLevel = 0.0, double capacityKg = 100.0);

    int getId() const;
    int getNodeId() const;
    const std::string& getLocation() const;
    double getFillLevel() const;
    double getCapacityKg() const;
    bool isCollected() const;
    double getWasteKg() const;
    double getPriority() const;

    void setFillLevel(double fillLevel);
    void setCollected(bool collected);

private:
    int id_;
    int nodeId_;
    std::string location_;
    double fillLevel_;
    double capacityKg_;
    bool collected_;
};

#endif // BIN_H
