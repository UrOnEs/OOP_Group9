#include "Game/Player.h"
#include "Entity System/Entity Type/Unit.h"     
#include "Entity System/Entity Type/Building.h"
#include <iostream>
#include <algorithm>

Player::Player() {
    // Baþlangýç kaynaklarý
    playerResources.add(ResourceType::Wood, 100);
    playerResources.add(ResourceType::Food, 100);
    playerResources.add(ResourceType::Gold, 0);
    playerResources.add(ResourceType::Stone, 50);

    // Limitler
    unitLimit = 5;
    buildLimit = 999;
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
            // EÐER BU BÝR BÝNAYSA ÇÝZME (MapManager zaten çiziyor)
            if (std::dynamic_pointer_cast<Building>(entity)) {
                // Ancak bina SEÇÝLÝYSE seçim halkasýný çizmemiz gerekebilir.
                // Entity::render içinde seçim halkasý mantýðý var.
                // Eðer burada çizmezsek seçim halkasý görünmeyebilir.

                // Seçenek A: Býrak iki kere çizilsin (Basit çözüm)
                // Seçenek B: Sadece seçiliyse çiz (Daha optimize)
                if (entity->isSelected) entity->render(window);

                continue;
            }
            entity->render(window);
        }
    }
}

void Player::removeDeadEntities() {
    entities.erase(
        std::remove_if(
            entities.begin(),
            entities.end(),
            [](const std::shared_ptr<Entity>& e) { return !e->getIsAlive(); }
        ),
        entities.end()
    );

    // Seçili olanlar listesinden de temizle (Ölen askeri seçili tutmayalým)
    selected_entities.erase(
        std::remove_if(
            selected_entities.begin(),
            selected_entities.end(),
            [](const std::shared_ptr<Entity>& e) { return !e->getIsAlive(); }
        ),
        selected_entities.end()
    );
}

// --- LÝMÝT VE SAYIM FONKSÝYONLARI (Eksik olanlar bunlardý) ---
int Player::getUnitLimit() {
    return unitLimit;
}

int Player::getUnitCount() {
    // Canlý olan entity sayýsýný (veya sadece asker sayýsýný) döndür
    int count = 0;
    for (auto& e : entities) {
        // 1. Nesne canlý mý?
        if (e->getIsAlive()) {

            // 2. Bu nesne bir "Unit" (Asker veya Köylü) mi?
            // Binalar (Building) Unit sýnýfýndan türemediði için bu kontrol onlarý saymaz.
            if (std::dynamic_pointer_cast<Unit>(e)) {
                count++;
            }
        }
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

// ====================================================================================
//                                SEÇÝM SÝSTEMÝ GÜNCELLEMESÝ
// ====================================================================================

// --- 1. TEKÝL SEÇÝM (TIKLAMA) ---
void Player::selectUnit(sf::RenderWindow& window, const sf::View& camera, bool isShiftHeld) {
    sf::Vector2i mousePosPixel = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosWorld = window.mapPixelToCoords(mousePosPixel, camera);

    bool clickedOnUnit = false;
    bool clickedOnBuilding = false;

    // --- ADIM 1: ÖNCE ASKERLERÝ (UNITS) KONTROL ET ---
    // Askerler binalarýn önünde durabilir, önce onlarý seçmek isteriz.
    for (auto& entity : entities) {
        if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {

            // Bu bir ASKER (Unit) mi?
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                // Shift basýlý DEÐÝLSE önceki seçimleri temizle
                if (!isShiftHeld && !clickedOnUnit) {
                    for (auto& e : selected_entities) e->setSelected(false);
                    selected_entities.clear();
                }

                // Seçimi yap/kaldýr
                if (isShiftHeld && unit->isSelected) {
                    // Zaten seçiliyse ve Shift varsa -> Çýkar (Toggle)
                    unit->setSelected(false);
                    // (Vektörden silme iþlemi biraz maliyetli, basit býrakýyoruz þimdilik)
                }
                else if (!unit->isSelected) {
                    unit->setSelected(true);
                    selected_entities.push_back(unit);
                }

                clickedOnUnit = true;
                break; // Bir asker bulduk, döngüden çýk (En üsttekini seç)
            }
        }
    }

    // --- ADIM 2: ASKER BULAMADIYSAK BÝNALARA BAK ---
    if (!clickedOnUnit) {
        for (auto& entity : entities) {
            if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {

                // Bu bir BÝNA (Building) mý?
                if (auto building = std::dynamic_pointer_cast<Building>(entity)) {

                    // KURAL: Bina seçerken her zaman tekli seçim yapýlýr.
                    // Shift basýlý olsa bile her þeyi temizle.
                    for (auto& e : selected_entities) e->setSelected(false);
                    selected_entities.clear();

                    // Binayý seç
                    building->setSelected(true);
                    selected_entities.push_back(building);

                    clickedOnBuilding = true;
                    break; // Binayý bulduk, çýk.
                }
            }
        }
    }

    // --- ADIM 3: BOÞA TIKLAMA ---
    // Hiçbir þeye (Asker veya Bina) týklamadýysak ve Shift yoksa her þeyi býrak
    if (!clickedOnUnit && !clickedOnBuilding && !isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }
}

// --- 2. ÇOKLU SEÇÝM (SÜRÜKLE-BIRAK / BOX SELECTION) ---
void Player::selectUnitsInRect(const sf::FloatRect& selectionRect, bool isShiftHeld) {

    // Shift basýlý DEÐÝLSE önce temizle
    if (!isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }

    for (auto& entity : entities) {
        if (!entity->getIsAlive()) continue;

        // --- DEÐÝÞÝKLÝK BURADA: BÝNALARI YOKSAY ---
        // Eðer bu entity bir Bina ise, kutu seçiminde onu es geç.
        if (std::dynamic_pointer_cast<Building>(entity)) {
            continue;
        }

        // Sadece Unit'ler buraya gelebilir
        if (selectionRect.intersects(entity->getBounds())) {
            if (!entity->isSelected) {
                entity->setSelected(true);
                selected_entities.push_back(entity);
            }
        }
    }
}