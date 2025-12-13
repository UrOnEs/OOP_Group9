#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class Farm : public ResourceGenerator {
public:
    Farm() {
        buildingType = BuildTypes::Farm;
        health = GameRules::HP_Farm; // Caný az

        interval = GameRules::Time_Harvest_Tick * 2.0f; // Tarla biraz daha yavaþ olsun (Örn: 2 saniye)
        amountPerTick = 20;
    }

    std::string getInfo() override { return "Farm: Yemek Uretir"; }
};