#pragma once
#include "Building.h"
#include "Game/GameRules.h" // GameRules eklendi

class House : public Building {
public:
    int populationBonus;

    House() {
        buildingType = BuildTypes::House;

        // GameRules'dan deðerleri çekiyoruz:
        health = GameRules::HP_House;

        populationBonus = 5;

        // Boyut/Alan bilgisi (Çarpýþma için ileride lazým olur)
        // setSize(GameRules::Size_House); 
    }

    // Class içine ekle:
    int getMaxHealth() const override { return (int)GameRules::HP_House; }

    std::string getInfo() override { return "House: +5 Nufus"; }
};