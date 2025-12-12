#pragma once
#include "Game/Player.h"
#include "Entity System/Entity Type/House.h"

class PopulationSystem {
public:
    static void updateLimit(Player& player) {
        int totalLimit = 10; // Baþlangýç limiti (Merkez binadan gelen)

        for (auto& entity : player.getEntities()) {
            // Entity bir House mu?
            if (auto house = std::dynamic_pointer_cast<House>(entity)) {
                if (house->getIsAlive()) {
                    totalLimit += house->populationBonus;
                }
            }
        }

        player.addUnitLimit(totalLimit); // Player'a setUnitLimit eklemelisin
    }
};
