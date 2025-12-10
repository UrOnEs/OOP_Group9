#pragma once

#include "Entity System/Entity.h"
#include <vector>

class Unit : public Entity
{
protected:
    float travelSpeed;
    float attackSpeed;

    bool isMoving;

    std::vector<sf::Vector2f> path;

public:
    // Hýz deðerine sistemlerin ulaþmasý lazým
    float getSpeed() const { return travelSpeed; }
};