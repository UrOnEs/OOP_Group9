#ifndef VILLAGER_H
#define VILLAGER_H

#include "Unit.h"
#include "types.h"
#include <vector>
#include <memory> 

class ResourceGenerator;
class Building;

class Villager : public Unit {
private:
    static int IDcounter;
    static int counter;

    bool isBusy = false;
    std::weak_ptr<ResourceGenerator> targetResource;
    bool isHarvesting = false;

public:
    Villager();
    ~Villager();

    std::string stats() override;

    void startHarvesting(std::shared_ptr<ResourceGenerator> resource);
    void stopHarvesting();

    // --- DEÐÝÞÝKLÝK BURADA ---
    // Artýk güncelleme yaparken etrafýna bakabilmesi için binalarý istiyoruz
    void updateHarvesting(const std::vector<std::shared_ptr<Building>>& buildings);

    void checkTargetStatus();
    void findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings);

    bool getIsHarvesting() const { return isHarvesting; }

    std::shared_ptr<ResourceGenerator> getTargetResource() {
        return targetResource.lock();
    }
};

#endif // !VILLAGER_H