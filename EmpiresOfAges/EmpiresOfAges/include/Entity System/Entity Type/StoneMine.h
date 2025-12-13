#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class StoneMine : public ResourceGenerator {
public:
    StoneMine() {
        buildingType = BuildTypes::StoneMine;
        health = GameRules::BuildingHealth; // Standart bina caný

        interval = GameRules::Time_Harvest_Tick; // Standart hýz
        amountPerTick = 5;
    }

    std::string getInfo() override { return "Stone Mine: Tas Uretir"; }
};