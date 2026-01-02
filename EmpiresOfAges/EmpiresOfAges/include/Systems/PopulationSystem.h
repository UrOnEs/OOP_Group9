#pragma once
#include "Game/Player.h"
#include "Entity System/Entity Type/House.h"
#include <memory>

/**
 * @brief Manages player population limits based on owned buildings.
 */
class PopulationSystem {
public:
    /**
     * @brief Recalculates and updates the unit limit for the player.
     * Iterates through player entities to count active houses.
     */
    static void updateLimit(Player& player) {
        int totalLimit = 10; // Base limit provided by Town Center

        for (auto& entity : player.getEntities()) {
            if (auto house = std::dynamic_pointer_cast<House>(entity)) {
                if (house->getIsAlive()) {
                    totalLimit += house->populationBonus;
                }
            }
        }

        player.setUnitLimit(totalLimit);
    }
};