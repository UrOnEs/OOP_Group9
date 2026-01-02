#pragma once
#include "Building.h"
#include "Game/GameRules.h" 
#include "Game/Player.h"

class House : public Building {
public:
    int populationBonus;

    House() {
        buildingType = BuildTypes::House;
        health = GameRules::HP_House;
        populationBonus = 5;
    }

    void onConstructionComplete(Player& owner) override {
        owner.addUnitLimit(populationBonus);
    }

    int getMaxHealth() const override { return (int)GameRules::HP_House; }
    std::string getInfo() override { return "House: +5 Population"; }
};