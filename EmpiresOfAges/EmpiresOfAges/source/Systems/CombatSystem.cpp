#include "Systems/CombatSystem.h"
#include "Game/GameRules.h"
#include <cmath>

bool CombatSystem::attack(Soldier& attacker, Entity& target) {
    sf::Vector2f diff = target.getPosition() - attacker.getPosition();
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    float targetRadius = target.getBounds().width / 2.0f;
    if (targetRadius < 15.0f) targetRadius = 15.0f;

    float attackerRange = attacker.getRange();
    float effectiveRange;

    // Range calculation adjustments based on unit type
    if (attackerRange > 30.0f) {
        // Ranged Unit: Use direct range
        effectiveRange = attackerRange + 20.0f;
    }
    else {
        // Melee Unit: Add target radius to get within touching distance
        effectiveRange = attackerRange + targetRadius + 20.0f;
    }

    if (distance <= effectiveRange) {
        target.takeDamage(attacker.damage);
        return true;
    }

    return false;
}