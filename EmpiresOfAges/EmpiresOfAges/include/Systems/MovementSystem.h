#pragma once
#include "Entity System/Entity Type/Unit.h"
#include <SFML/System/Vector2.hpp>

/**
 * @brief Handles unit movement logic and path traversal.
 */
class MovementSystem {
public:
    /**
     * @brief Updates the unit's position towards the next waypoint in its path.
     * @param unit The unit to move.
     * @param target (Unused/Legacy) Target is determined by the unit's internal path.
     * @param deltaTime Time elapsed since last frame.
     * @return true if the unit has reached its final destination.
     */
    static bool move(Unit& unit, const sf::Vector2f& target, float deltaTime);
};