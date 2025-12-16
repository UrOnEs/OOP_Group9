#include "Game/Player.h"
#include <iostream>

Player::Player() {
    // Baþlangýç kaynaklarý
    playerResources.add(ResourceType::Wood, 200);
    playerResources.add(ResourceType::Food, 200);
    playerResources.add(ResourceType::Gold, 100);
    playerResources.add(ResourceType::Stone, 0);

    // Limitler
    unitLimit = 10;
    buildLimit = 5;
}

Player::~Player() {
    entities.clear();
}

// --- KAYNAK YÖNETÝMÝ ---
std::vector<int> Player::getResources() {
    // ResourceManager'dan verileri çekip vektör olarak döndür
    return {
        playerResources.getAmount(ResourceType::Wood),
        playerResources.getAmount(ResourceType::Gold),
        playerResources.getAmount(ResourceType::Stone),
        playerResources.getAmount(ResourceType::Food)
    };
}

// --- ENTITY YÖNETÝMÝ ---
std::vector<std::shared_ptr<Entity>> Player::getEntities() {
    return entities;
}

void Player::renderEntities(sf::RenderWindow& window) {
    for (auto& entity : entities) {
        if (entity->getIsAlive()) {
            entity->render(window);
        }
    }
}

// --- LÝMÝT VE SAYIM FONKSÝYONLARI (Eksik olanlar bunlardý) ---
int Player::getUnitLimit() {
    return unitLimit;
}

int Player::getUnitCount() {
    // Canlý olan entity sayýsýný (veya sadece asker sayýsýný) döndür
    int count = 0;
    for (auto& e : entities) {
        if (e->getIsAlive()) count++;
    }
    return count;
}

bool Player::addUnitLimit(int amount) {
    unitLimit += amount;
    return true;
}

bool Player::setUnitLimit(int amount) {
    unitLimit = amount;
    return true;
}

// --- BÝNA VE ÜRETÝM SÝSTEMLERÝ ÝÇÝN YARDIMCI FONKSÝYONLAR ---
// Header'da inline (süslü parantez içinde) tanýmladýysan burada tekrar yazmana gerek yok.
// Ama hata alýyorsan, header'dakini silip buraya ekleyebilirsin.
// Þimdilik Player.h içinde tanýmlý olduklarýný varsayýyorum.
// Eðer "addWood already defined" derse buraya dokunma.
// Eðer "addWood unresolved" derse aþaðýdakileri yorumdan çýkar:

/*
void Player::addWood(int amount) { playerResources.add(ResourceType::Wood, amount); }
void Player::addGold(int amount) { playerResources.add(ResourceType::Gold, amount); }
void Player::addFood(int amount) { playerResources.add(ResourceType::Food, amount); }
void Player::addStone(int amount) { playerResources.add(ResourceType::Stone, amount); }
void Player::addEntity(std::shared_ptr<Entity> entity) { entities.push_back(entity); }
*/

// --- SEÇÝM SÝSTEMÝ ---
std::vector<std::shared_ptr<Entity>> Player::selectUnit(sf::RenderWindow& window, const sf::View& camera) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        selected_entities.clear();
        sf::Vector2i mousePosPixel = sf::Mouse::getPosition(window);

        // --- DÜZELTME BURADA ---
        // Eskiden: window.mapPixelToCoords(mousePosPixel); 
        // Þimdi kamerayý (view) parametre olarak veriyoruz:
        sf::Vector2f mousePosWorld = window.mapPixelToCoords(mousePosPixel, camera);

        for (auto& entity : entities) {
            // entity->getBounds() zaten dünya koordinatlarýnda olduðu için artýk eþleþecekler.
            if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {
                selected_entities.push_back(entity);
                entity->setSelected(true);
            }
            else {
                // Çoklu seçim (Shift tuþu vs.) yoksa diðerlerini iptal et
                entity->setSelected(false);
            }
        }
    }
    return selected_entities;
}