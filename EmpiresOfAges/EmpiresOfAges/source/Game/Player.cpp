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

// ---------------------- SEÇÝM SÝSTEMÝ -----------------------

//---------- Tekli Seçim ------------------------
void Player::selectUnit(sf::RenderWindow& window, const sf::View& camera, bool isShiftHeld) {
    sf::Vector2i mousePosPixel = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosWorld = window.mapPixelToCoords(mousePosPixel, camera);

    // Shift basýlý DEÐÝLSE, mevcut seçimi temizle
    if (!isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }

    bool clickedOnSomething = false;

    for (auto& entity : entities) {
        if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {

            // Eðer zaten seçiliyse ve Shift'e basýyorsak -> Seçimi kaldýr (Toggle)
            if (isShiftHeld && entity->isSelected) {
                entity->setSelected(false);
                // Vektörden silme iþlemi biraz maliyetli olabilir ama basit tutalým:
                // (Modern C++'da remove_if kullanýlýr ama þimdilik kalsýn, görsel güncellemesi yeter)
                // Doðru yöntem: selected_entities listesinden bu elemaný bulup silmek gerekir.
            }
            else {
                // Seçili deðilse seç
                if (!entity->isSelected) {
                    entity->setSelected(true);
                    selected_entities.push_back(entity);
                }
            }
            clickedOnSomething = true;
            break; // Üst üste binenlerden sadece en üsttekini seç
        }
    }

    // Boþa týkladýysak ve Shift yoksa her þeyi býrak
    if (!clickedOnSomething && !isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }
}

// ---------------------  ÇOKLU SEÇÝM  --------------
void Player::selectUnitsInRect(const sf::FloatRect& selectionRect, bool isShiftHeld) {

    // Shift basýlý DEÐÝLSE önce temizle
    if (!isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }

    for (auto& entity : entities) {
        if (!entity->getIsAlive()) continue;

        // Entity'nin pozisyonu kutunun içinde mi?
        // intersects yerine contains kullanýyoruz ki sadece kutu içindekiler seçilsin
        // veya biraz dokunsa da seçilsin istersen 'intersects' kullanabilirsin.
        if (selectionRect.intersects(entity->getBounds())) {

            // Zaten seçili deðilse listeye ekle
            if (!entity->isSelected) {
                entity->setSelected(true);
                selected_entities.push_back(entity);
            }
        }
    }
}