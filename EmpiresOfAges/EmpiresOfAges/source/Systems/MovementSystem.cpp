
/*"Systems/MovementSystem.h"
#include <cmath>
#include <iostream>

bool MovementSystem::move(Unit& unit, const sf::Vector2f& ignoredTarget, float deltaTime) {
    // NOT: ignoredTarget parametresini kullanmýyoruz çünkü unit.path listesini takip edeceðiz.

    // Eðer gidecek yol yoksa dur.
    if (unit.path.empty()) return true; // Hedefe vardý

    // Sýradaki hedef (Listenin baþý)
    sf::Vector2f nextWaypoint = unit.path[0];
    sf::Vector2f currentPos = unit.getPosition();
    sf::Vector2f direction = nextWaypoint - currentPos;

    // Uzaklýk hesapla
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Bir sonraki kareye çok yaklaþtýysa (Örn: 2 piksel)
    if (distance < 2.0f) {
        unit.setPosition(nextWaypoint); // Tam oturt
        unit.path.erase(unit.path.begin()); // O noktayý listeden sil (Sýradakine geç)

        // Eðer yol bittiyse durdu demektir
        return unit.path.empty();
    }

    // --- HAREKET ---
    sf::Vector2f normalizedDir = direction / distance;
    sf::Vector2f moveAmount = normalizedDir * unit.getSpeed() * deltaTime;

    unit.move(moveAmount);

    return false; // Hala yolda
}
*/