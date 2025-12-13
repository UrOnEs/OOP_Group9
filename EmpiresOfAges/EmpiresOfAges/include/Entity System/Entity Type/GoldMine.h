#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class GoldMine : public ResourceGenerator {
public:
    GoldMine() {
        buildingType = BuildTypes::GoldMine;
        health = GameRules::BuildingHealth;

        interval = GameRules::Time_Harvest_Tick;
        amountPerTick = 5;
    }

    std::string getInfo() override { return "Gold Mine: Altin Uretir"; }
};