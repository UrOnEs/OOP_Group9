#include "Systems/CombatSystem.h"
#include "Game/GameRules.h"
#include <cmath>

bool CombatSystem::attack(Soldier& attacker, Entity& target) {
    // Mesafe kontrolü
    sf::Vector2f diff = target.getPosition() - attacker.getPosition();
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    if (distance <= attacker.getRange()) {
        // Hasar ver
        target.takeDamage(GameRules::Dmg_Barbarian); // Þimdilik sabit hasar
        return true;
    }
    return false; // Menzil dýþý
}