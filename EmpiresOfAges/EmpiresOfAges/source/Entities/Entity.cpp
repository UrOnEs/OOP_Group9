#include "Entity System/Entity.h"

// Entity'nin kendini ekrana çizme fonksiyonu
void Entity::render(sf::RenderWindow& window) {
    if (isAlive) {
        window.draw(model);

        // Eðer seçiliyse, altýna veya üstüne bir iþaret koyabilirsin (Ýleride)
        if (isSelected) {
            // window.draw(selectionCircle);
        }
    }
}