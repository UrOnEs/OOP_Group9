#pragma once
#include "Building.h"

class TownCenter : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;

public:
    TownCenter() {
        buildingType = BuildTypes::TownCenter;
        health = GameRules::HP_TownCenter;
        // 3x3 olduðu için merkez noktasý daha geniþ
        this->sprite.setOrigin(96.f, 150.f);
    }

    // Class içine ekle:
    int getMaxHealth() const override { return (int)GameRules::HP_TownCenter; }

    void startProduction() {
        if (!isProducing) {
            productionTimer = GameRules::Time_Build_Villager;
            isProducing = true;
        }
    }


    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    bool isReady() const { return isProducing && productionTimer <= 0; }
    void finishProduction() { isProducing = false; }

    std::string getInfo() override { return "TownCenter: Ana Merkez"; }
};
