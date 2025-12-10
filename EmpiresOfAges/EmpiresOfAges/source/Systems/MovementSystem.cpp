#include "Systems/MovementSystem.h"
#include <cmath> // sqrt ve hypot için

bool MovementSystem::move(Unit& unit, const sf::Vector2f& target, float deltaTime) {
    sf::Vector2f currentPos = unit.getPosition();
    sf::Vector2f direction = target - currentPos;

    // Uzaklýk hesapla (Magnitude)
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Hedefe çok yakýnsa durdur (titremeyi önler)
    if (distance < 1.0f) {
        unit.setPosition(target);
        return true; // Hedefe vardý
    }

    // Vektörü normalize et (Birim vektör yap)
    sf::Vector2f normalizedDir = direction / distance;

    // Yeni pozisyon = Eski Pozisyon + (Yön * Hýz * Zaman)
    sf::Vector2f newPos = currentPos + (normalizedDir * unit.getSpeed() * deltaTime);

    unit.setPosition(newPos);
    return false; // Hala hareket ediyor
}