#pragma once
#include "Building.h"
#include "Game/GameRules.h" // GameRules eklendi
#include "Game/Player.h"

class House : public Building {
private:
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
    void onConstructionComplete(Player& owner) override {
        owner.addUnitLimit(populationBonus); // Nüfus sadece bina bitince eklenir!
        std::cout << "[HOUSE] Insaat bitti! Nufus +5 eklendi.\n";
    }
    // Class içine ekle:
    int getMaxHealth() const override { return (int)GameRules::HP_House; }

    std::string getInfo() override { return "House: +5 Nufus"; }
};