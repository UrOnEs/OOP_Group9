#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/Building.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/ResourceGenerator.h" // ResourceGenerator metodlarýný kullanmak için þart
#include <iostream>
#include <cmath> // sqrt için

// Static üyeleri baþlat
int Villager::IDcounter = 0;
int Villager::counter = 0;

Villager::Villager() : Unit() {
    // GameRules'dan verileri çek
    this->health = GameRules::HP_Villager;
    this->damage = GameRules::Dmg_Villager;
    this->travelSpeed = GameRules::Speed_Villager;
    this->range = GameRules::Range_Melee;

    this->isAlive = true;
    this->isSelected = false;
    this->entityID = ++IDcounter;

    counter++;
}

Villager::~Villager() {
    counter--;
}

std::string Villager::stats() {
    return "Villager HP: " + std::to_string((int)health);
}

// =========================================================
//               HASAT VE HAREKET MANTIÐI
// =========================================================

void Villager::startHarvesting(ResourceGenerator* resource) {
    if (!resource) return;

    targetResource = resource;
    isHarvesting = true;

    // Aðacýn merkezine doðru yürü
    moveTo(resource->getPosition());
}

void Villager::stopHarvesting() {
    if (isHarvesting && targetResource) {
        // Eðer içerideysek dýþarý çýkalým
        targetResource->releaseWorker();
    }
    isHarvesting = false;
    targetResource = nullptr;
}

// Bu fonksiyon .h dosyasýnda sadece tanýmlýydý, gövdesi burada olmalý:
void Villager::checkTargetStatus() {
    // Eðer hasat yapýyorsak ama hedef kaynak yok olduysa (isAlive == false)
    if (isHarvesting && targetResource) {
        if (!targetResource->getIsAlive()) {
            stopHarvesting(); // Pointer'ý sýfýrla ki oyun çökmesin
        }
    }
}

void Villager::updateHarvesting() {
    if (!isHarvesting || !targetResource) return;

    // Hedef hala yaþýyor mu? (Ekstra güvenlik)
    if (!targetResource->getIsAlive()) {
        stopHarvesting();
        return;
    }

    // Mesafeyi hesapla
    sf::Vector2f diff = targetResource->getPosition() - getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    // --- 50 BÝRÝM MESAFE KONTROLÜ ---
    // Köylü aðaca yeterince yaklaþtý mý?
    if (dist <= 50.0f) {

        // Aðaçta yer varsa (baþkasý çalýþmýyorsa)
        if (!targetResource->isWorking()) {

            // Ýçeri gir / Ýþe baþla
            if (targetResource->garrisonWorker()) {
                std::cout << "[Villager] Agaca ulasti (Mesafe: " << dist << "), kesim basladi!\n";

                // Artýk yürümeyi durdur
                m_path.clear();
                m_isMoving = false;
            }
        }
    }
}

void Villager::findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings) {
    float minDistance = 500.0f; // Arama yarýçapý (Örn: 500 piksel etrafýna bakar)
    ResourceGenerator* closest = nullptr;

    for (const auto& building : buildings) {
        // 1. Bina var mý ve ResourceGenerator tipinde mi?
        if (building) {
            // Sadece Aðaçlarý (veya madenleri) arýyoruz
            // (Eðer sadece aðaç istiyorsan: building->buildingType == BuildTypes::Tree kontrolü ekle)
            if (building->buildingType == BuildTypes::Tree) {

                // 2. Dynamic Cast ile ResourceGenerator özelliklerine eriþ
                if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {

                    // 3. Bu kaynak yaþýyor mu? (Ölü aðaçlarý seçme)
                    // Ayrýca þu an kestiðimiz (ve az önce ölen) aðacý tekrar seçme.
                    if (res->getIsAlive() && res.get() != targetResource) {

                        // 4. Mesafeyi ölç
                        sf::Vector2f diff = res->getPosition() - getPosition();
                        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                        // 5. En yakýný güncelle
                        if (dist < minDistance) {
                            minDistance = dist;
                            closest = res.get();
                        }
                    }
                }
            }
        }
    }

    // Eðer uygun bir aðaç bulunduysa ona git
    if (closest) {
        std::cout << "[Villager] Yeni agac bulundu! Oraya gidiliyor...\n";
        startHarvesting(closest);
    }
    else {
        std::cout << "[Villager] Yakinda baska agac yok. Bekliyor.\n";
        stopHarvesting(); // Kimse yoksa dur
    }
}