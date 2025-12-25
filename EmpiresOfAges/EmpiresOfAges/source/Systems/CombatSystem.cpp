#include "Systems/CombatSystem.h"
#include "Game/GameRules.h"
#include <cmath>
#include <iostream> // Loglar için

bool CombatSystem::attack(Soldier& attacker, Entity& target) {
    // Mesafe hesapla
    sf::Vector2f diff = target.getPosition() - attacker.getPosition();
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    // Hedefin yarýçapý
    float targetRadius = target.getBounds().width / 2.0f;
    if (targetRadius < 15.0f) targetRadius = 15.0f;

    // Attacker.getRange() -> Unit sýnýfýndaki 'range' deðiþkenini getirir.
    float attackerRange = attacker.getRange();

    // Geçerli Menzil
    float effectiveRange = attackerRange + targetRadius + 20.0f;


    if (distance <= effectiveRange) {
        target.takeDamage(attacker.damage); // Askerin hasarýný kullan
        return true;
    }

    // Eðer buraya düþtüyse mesafe yetmiyor demektir. Sebebini yazdýr:
    std::cout << "[COMBAT FAIL] Mesafe: " << distance << " > Menzil: " << effectiveRange << "\n";
    return false;
}