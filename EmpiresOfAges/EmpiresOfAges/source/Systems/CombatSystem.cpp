#include "Systems/CombatSystem.h"
#include "Game/GameRules.h"
#include <cmath>
#include <iostream>

bool CombatSystem::attack(Soldier& attacker, Entity& target) {
    sf::Vector2f diff = target.getPosition() - attacker.getPosition();
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    float targetRadius = target.getBounds().width / 2.0f;
    if (targetRadius < 15.0f) targetRadius = 15.0f;

    float attackerRange = attacker.getRange();

    // --- MENZÝL HESABI DÜZELTMESÝ ---
    float effectiveRange;

    // Uzakçý mý (Range > 30) ?
    if (attackerRange > 30.0f) {
        // Uzakçý: Direkt menzili kullan (Çemberle uyumlu)
        effectiveRange = attackerRange + 20.0f;
    }
    else {
        // Yakýncý: Hedefin cüssesini ekle (Duvarýna kadar yürü)
        effectiveRange = attackerRange + targetRadius + 20.0f;
    }

    if (distance <= effectiveRange) {
        target.takeDamage(attacker.damage);
        return true;
    }

    // std::cout << "[COMBAT FAIL] Mesafe yetmedi.\n";
    return false;
}