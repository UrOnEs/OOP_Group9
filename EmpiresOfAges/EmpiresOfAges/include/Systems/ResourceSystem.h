#pragma once
#include "Game/Player.h"
#include "Entity System/Entity Type/ResourceGenerator.h"

/**
 * @brief Manages resource generation and updates player resources.
 */
class ResourceSystem {
public:
    /**
     * @brief Updates a resource generator building and credits the player.
     * @param player The player to receive resources.
     * @param building The resource generator entity.
     * @param dt Time delta.
     */
    static void update(Player& player, ResourceGenerator& building, float dt) {

        int producedAmount = building.updateGeneration(dt);

        if (producedAmount > 0) {
            if (building.buildingType == BuildTypes::Farm) {
                player.addFood(producedAmount);
            }
            else if (building.buildingType == BuildTypes::StoneMine) {
                player.addStone(producedAmount);
            }
            else if (building.buildingType == BuildTypes::Tree) {
                player.addWood(producedAmount);
            }
        }
    }
};