#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class Stone : public ResourceGenerator {
public:
    Stone() {
        buildingType = BuildTypes::Stone;
        health = GameRules::HP_Stone;
        interval = GameRules::Time_Harvest_Tick;
        amountPerTick = GameRules::Stone_Per_Tick;
        setTotalResources(GameRules::Resources_Per_Stone);
    }

    int getMaxHealth() const override { return (int)GameRules::HP_Stone; }
    std::string getInfo() override { return "Stone: Stone Source"; }
};