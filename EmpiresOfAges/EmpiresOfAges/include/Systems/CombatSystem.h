#pragma once

#include "Entity System/Entity Type/Soldier.h"
#include "Entity System/Entity.h"

class CombatSystem {
public:
    static bool attack(Soldier& attacker, Entity& target);
};
