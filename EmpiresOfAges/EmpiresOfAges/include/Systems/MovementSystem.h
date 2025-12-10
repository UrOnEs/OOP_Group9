#pragma once

#include "Entity System/Entity Type/Unit.h"
#include <SFML/System/Vector2.hpp>

class MovementSystem {
public:
    static bool move(Unit& unit, const sf::Vector2f& target, float deltaTime);
};