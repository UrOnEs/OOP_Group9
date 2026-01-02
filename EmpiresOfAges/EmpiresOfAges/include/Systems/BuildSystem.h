#pragma once
#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/Building.h"
#include <SFML/System/Vector2.hpp>

class Player;

/**
 * @brief Static system responsible for handling building construction logic.
 */
class BuildSystem {
public:
    /**
     * @brief Attempts to construct a building at the specified position.
     * Checks for resources and space availability before placement.
     * @param player The owner of the building.
     * @param worker The villager constructing the building.
     * @param building The building entity to be placed.
     * @param pos The target world coordinates.
     * @return true if construction started successfully, false otherwise.
     */
    static bool build(Player& player, Villager& worker, Building& building, const sf::Vector2f& pos);

private:
    /**
     * @brief Checks if the target area is free of obstacles.
     */
    static bool checkSpace(Building& building, const sf::Vector2f& pos);
};