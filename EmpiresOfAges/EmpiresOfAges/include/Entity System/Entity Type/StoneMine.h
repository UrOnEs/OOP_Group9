#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class StoneMine : public ResourceGenerator {
public:
    StoneMine() {
        buildingType = BuildTypes::StoneMine;
        health = GameRules::BuildingHealth;
        interval = GameRules::Time_Harvest_Tick;
        amountPerTick = 5;
    }
    std::string getInfo() override { return "Stone Mine"; }
};