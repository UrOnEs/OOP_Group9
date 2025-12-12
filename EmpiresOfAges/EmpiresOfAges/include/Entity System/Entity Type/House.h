#pragma once
#include "Building.h"

class House : public Building {
public:
    int populationBonus; // Bu ev kaç kiþilik yer açýyor?

    House() {
        buildingType = BuildTypes::House; // GameRules'a House eklemeyi unutma
        populationBonus = 5; // Her ev +5 limit
    }

    std::string getInfo() override { return "House: +5 Nufus Limiti"; }
};