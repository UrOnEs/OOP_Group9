#include "Game/Player.h"
#include "Game/ResourceManager.h"
#include <vector>
#include <iostream>

Player::Player() {
    // Oyuncuya baþlangýç kaynaðý verelim
    playerResources.add(ResourceType::Wood, 200);
    playerResources.add(ResourceType::Food, 200);

    // networkID = ... (LAN entegrasyonunda burasý deðiþecek)
}

Player::~Player() {
    entities.clear();
}

std::vector<int> Player::getResources() {
    return {
        playerResources.getAmount(ResourceType::Wood),
        playerResources.getAmount(ResourceType::Gold),
        playerResources.getAmount(ResourceType::Stone),
        playerResources.getAmount(ResourceType::Food)
    };
}

void Player::renderEntities(sf::RenderWindow& window) {
    for (auto& entity : entities) {
        if (entity->getIsAlive()) {
            entity->render(window);
        }
    }
}

std::vector<std::shared_ptr<Entity>> Player::selectUnit(sf::RenderWindow& window) {

    // 1. Mouse'un sol tuþuna basýldý mý kontrol et
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

        // 2. Önceki seçimleri temizle (Çoklu seçim yapmýyorsak - Shift tuþu hariç)
        selected_entities.clear();

        // 3. Mouse'un ekran üzerindeki koordinatýný al
        sf::Vector2i mousePosPixel = sf::Mouse::getPosition(window);

        // 4. Bu koordinatý "Dünya Koordinatýna" çevir
        // (Bu çok önemli! Kamera hareket edince bozulmamasý için bunu yapmalýyýz)
        sf::Vector2f mousePosWorld = window.mapPixelToCoords(mousePosPixel);

        // 5. Tüm askerleri (entities) döngüye sokup kontrol et
        for (auto& entity : entities) {

            // Eðer ünite hayattaysa ve Mouse'un olduðu yer ünitenin sýnýrlarý içindeyse
            if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {

                // Seçilenlere ekle
                selected_entities.push_back(entity);

                // Görsel olarak seçildiðini belli et (Entity içine yazdýðýmýz varsayýlan metod)
                entity->setSelected(true);

                // Tekli seçim istiyorsak ilk bulduðumuzda döngüyü kýrabiliriz
                // break; 
            }
            else {
                // Týklanmayanlarýn seçimini kaldýr
                entity->setSelected(false);
            }
        }
    }

    return selected_entities;
}