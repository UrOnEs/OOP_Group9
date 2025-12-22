#ifndef VILLAGER_H
#define VILLAGER_H

#include "Unit.h"
#include "types.h"
#include <vector>  // vector için
#include <memory>  // shared_ptr için

// Forward Declaration
class ResourceGenerator;
class Building; // <-- Bunu ekle

class Villager : public Unit {
private:
    static int IDcounter;
    static int counter;

    bool isBusy = false;

    // --- HASAT DEÐÝÞKENLERÝ ---
    ResourceGenerator* targetResource = nullptr;
    bool isHarvesting = false;

public:
    Villager();
    ~Villager();

    std::string stats() override;

    void startHarvesting(ResourceGenerator* resource);
    void stopHarvesting();
    void updateHarvesting();
    void checkTargetStatus();

    // --- YENÝ EKLENEN FONKSÝYON ---
    // Etraftaki yeni kaynaðý bulur ve ona yönelir
    void findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings);

    // Getterlar
    bool getIsHarvesting() const { return isHarvesting; }
    ResourceGenerator* getTargetResource() { return targetResource; }
};

#endif // !VILLAGER_H