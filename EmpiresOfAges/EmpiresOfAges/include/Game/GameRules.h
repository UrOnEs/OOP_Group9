#pragma once

#include <SFML/System/Vector2.hpp>

namespace GameRules {

    // === COSTS ===
    constexpr int SoldierCost_Wood = 100;
    constexpr int VillagerCost_Wood = 50;

    // === HEALTH ===   
    constexpr float SoldierHealth = 100.f;
    constexpr float VillagerHealth = 20.f;
    constexpr float BuildingHealth = 500.f;

    // === DAMAGE ===
    constexpr float SoldierDamage = 20.f;
    constexpr float VillagerDamage = 2.f;

    // === SPEED ===
    constexpr float SoldierSpeed = 3.5f;
    constexpr float VillagerSpeed = 2.5f;

    // === RANGE ===
    constexpr float MeleeRange = 1.5f;
    constexpr float DistantRange = 5.0f;

    // === BUILD SPACES ===
    const sf::Vector2f NormalBuildingSpace = sf::Vector2f(5.f, 5.f);
    const sf::Vector2f BigBuildingSpace = sf::Vector2f(10.f, 10.f);
}
