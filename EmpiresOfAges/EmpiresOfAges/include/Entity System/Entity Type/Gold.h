#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class Gold : public ResourceGenerator {
public:
    Gold() {
        buildingType = BuildTypes::Gold;
        health = GameRules::HP_Gold;

        // Toplama hýzý
        interval = GameRules::Time_Harvest_Tick;
        amountPerTick = GameRules::Gold_Per_Tick;

        // Altýnýn içindeki toplam kaynak
        setTotalResources(GameRules::Resources_Per_Gold);
    }

    int getMaxHealth() const override { return (int)GameRules::HP_Gold; }

    std::string getInfo() override { return "Gold: Altin Kaynagi"; }
};