#pragma once
#include "Building.h"
#include "Soldier.h"

class Barracks : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f;
    SoldierTypes currentProduction;

public:
    Barracks() {
        buildingType = BuildTypes::Barrack;
        
        // Kýþla daha saðlamdýr
        health = GameRules::HP_Barracks;
    }

    // Üretimi Baþlat
    void startTraining(SoldierTypes type, float duration) {
        if (!isProducing) {
            currentProduction = type;
            productionTimer = duration;
            productionDuration = duration; // Ýleride progress bar yapmak istersen lazým olur
            isProducing = true;
        }
    }

    // Süreyi Güncelle (ProductionSystem çaðýracak)
    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    // Bitti mi?
    bool isReady() const { return isProducing && productionTimer <= 0; }

    // Üretimi Bitir
    SoldierTypes finishTraining() {
        isProducing = false;
        return currentProduction;
    }

    bool getIsProducing() const { return isProducing; }

    std::string getInfo() override { return "Barracks: Asker Uretir"; }
};
