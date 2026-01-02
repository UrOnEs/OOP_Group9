#pragma once
#include "Entity System/Entity Type/Soldier.h"
#include "Entity System/Entity.h"

/**
 * @brief Handles combat interactions between entities.
 */
class CombatSystem {
public:
    /**
     * @brief Calculates attack logic from a soldier to a target entity.
     * Checks range and applies damage if valid.
     * @return true if the attack was successfully performed.
     */
    static bool attack(Soldier& attacker, Entity& target);
};