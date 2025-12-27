#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/Building.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/ResourceGenerator.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Map/PathFinder.h"
#include <iostream>
#include <cmath>

int Villager::IDcounter = 0;
int Villager::counter = 0;

Villager::Villager() : Unit() {
    this->health = GameRules::HP_Villager;
    this->damage = GameRules::Dmg_Villager;
    this->travelSpeed = GameRules::Speed_Villager;
    this->range = GameRules::Range_Melee;
    this->isAlive = true;
    this->isSelected = false;
    this->entityID = ++IDcounter;
    this->setTexture(AssetManager::getTexture("assets/units/default.png"));

    float scaleX = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().x;
    float scaleY = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().y;
    this->setScale(scaleX, scaleY);

    // Yetenekler
    addAbility(Ability(1, "Ev Insa Et", "30 Odun", "+5 Nufus", &AssetManager::getTexture("assets/buildings/house.png")));
    addAbility(Ability(2, "Kisla Yap", "175 Odun", "Asker Uret", &AssetManager::getTexture("assets/buildings/barrack.png")));
    addAbility(Ability(3, "Ciftlik Kur", "60 Odun", "Yemek Uret", &AssetManager::getTexture("assets/buildings/mill.png")));
    addAbility(Ability(4, "Ana bina", "400 Odun", "Merkez", &AssetManager::getTexture("assets/buildings/castle.png")));

    counter++;
}

Villager::~Villager() { counter--; }

std::string Villager::stats() {
    return "Yuk: " + std::to_string(currentCargo) + "/" + std::to_string(maxCargo);
}

int Villager::getMaxHealth() const { return GameRules::HP_Villager; }

// --- HASAT BAÞLATMA ---
void Villager::startHarvesting(std::shared_ptr<ResourceGenerator> resource) {
    if (!resource) return;

    targetResource = resource;
    state = VillagerState::MovingToResource;

    // Kaynak tipini belirle
    if (resource->buildingType == BuildTypes::Tree) cargoType = ResourceType::Wood;
    else if (resource->buildingType == BuildTypes::GoldMine) cargoType = ResourceType::Gold; // Bina maden
    else if (resource->buildingType == BuildTypes::StoneMine) cargoType = ResourceType::Stone;
    else if (resource->buildingType == BuildTypes::Stone) cargoType = ResourceType::Stone;
    else if (resource->buildingType == BuildTypes::Gold) cargoType = ResourceType::Gold; // <-- YENÝ: Yerdeki Altýn
    else if (resource->buildingType == BuildTypes::Farm) cargoType = ResourceType::Food;

    moveTo(resource->getPosition());
}

// --- HASAT DURDURMA ---
void Villager::stopHarvesting() {
    if (auto res = targetResource.lock()) {
        res->releaseWorker();
    }
    state = VillagerState::Idle;
    targetResource.reset();
    targetBase.reset();
    m_path.clear();
    m_isMoving = false;
}

// --- ANA DÖNGÜ (State Machine) ---
void Villager::updateVillager(float dt, const std::vector<std::shared_ptr<Building>>& buildings, Player& player) {

    // 0. Durum: GARRISONED (Binanýn Ýçinde - Saklanmýþ)
    if (state == VillagerState::Garrisoned) {
        auto res = targetResource.lock();
        if (!res || !res->getIsAlive()) {
            state = VillagerState::Idle;
            this->setPosition(this->getPosition() + sf::Vector2f(0, 50));
        }
        return;
    }

    // 1. Durum: KAYNAÐA GÝDÝYOR
    if (state == VillagerState::MovingToResource) {
        auto res = targetResource.lock();

        if (!res || !res->getIsAlive()) {
            findNearestResource(buildings);
            return;
        }

        sf::Vector2f diff = res->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        float targetRadius = res->getBounds().width / 2.0f;
        if (targetRadius < 32.0f) targetRadius = 32.0f;

        float interactionDist = targetRadius + 15.0f;

        if (dist <= interactionDist) {
            if (res->buildingType == BuildTypes::Farm) {
                if (res->garrisonWorker()) {
                    state = VillagerState::Garrisoned;
                    m_path.clear();
                    m_isMoving = false;
                    isSelected = false;
                }
                else {
                    stopHarvesting();
                }
            }
            // --- NORMAL KAYNAK (AÐAÇ/MADEN/TAÞ) ---
            else {
                if (res->garrisonWorker()) {
                    state = VillagerState::Harvesting;
                    m_path.clear();
                    m_isMoving = false;
                }
                else {
                    findNearestResource(buildings);
                }
            }
        }
    }

    // 2. Durum: KAYNAK TOPLUYOR (Harvesting)
    else if (state == VillagerState::Harvesting) {
        auto res = targetResource.lock();

        if (!res || !res->getIsAlive()) {
            if (currentCargo > 0) {
                state = VillagerState::ReturningToBase;
            }
            else {
                findNearestResource(buildings);
            }
            return;
        }

        int amount = res->updateGeneration(dt);
        if (amount > 0) {
            currentCargo += amount;

            if (currentCargo >= maxCargo) {
                currentCargo = maxCargo;
                res->releaseWorker();

                auto base = findNearestBase(buildings);
                if (base) {
                    targetBase = base;
                    state = VillagerState::ReturningToBase;
                    moveTo(base->getPosition());
                }
                else {
                    std::cout << "[UYARI] Depo yok! Bekliyorum.\n";
                    stopHarvesting();
                }
            }
        }
    }

    // 3. Durum: EVE DÖNÜYOR (Returning)
    else if (state == VillagerState::ReturningToBase) {
        auto base = targetBase.lock();

        if (!base || !base->getIsAlive()) {
            base = findNearestBase(buildings);
            if (base) {
                targetBase = base;
                moveTo(base->getPosition());
            }
            return;
        }

        sf::Vector2f diff = base->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        float buildingRadius = base->getBounds().width / 2.0f;
        if (buildingRadius < 32.0f) buildingRadius = 32.0f;
        float interactionRange = buildingRadius + 50.0f;

        if (dist <= interactionRange) {
            // Boþalt
            if (cargoType == ResourceType::Wood) player.addWood(currentCargo);
            else if (cargoType == ResourceType::Food) player.addFood(currentCargo);
            else if (cargoType == ResourceType::Gold) player.addGold(currentCargo);
            else if (cargoType == ResourceType::Stone) player.addStone(currentCargo);

            currentCargo = 0;

            // Tekrar Kaynaða Dön
            auto res = targetResource.lock();
            if (res && res->getIsAlive() && !res->isFull()) {
                state = VillagerState::MovingToResource;
                moveTo(res->getPosition());
            }
            else {
                findNearestResource(buildings);
            }
        }
    }
}

// --- EN YAKIN DEPOYU BUL ---
std::shared_ptr<Building> Villager::findNearestBase(const std::vector<std::shared_ptr<Building>>& buildings) {
    float minDist = 100000.f;
    std::shared_ptr<Building> nearest = nullptr;

    for (const auto& b : buildings) {
        if (!b->getIsAlive()) continue;

        if (b->buildingType == BuildTypes::TownCenter) {
            float d = PathFinder::heuristic({ (int)getPosition().x, (int)getPosition().y },
                { (int)b->getPosition().x, (int)b->getPosition().y });
            if (d < minDist) {
                minDist = d;
                nearest = b;
            }
        }
    }
    return nearest;
}

// --- EN YAKIN KAYNAK BUL (GÜNCELLENMÝÞ) ---
void Villager::findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings) {
    float minDistance = 2000.0f; // Arama menzili
    std::shared_ptr<ResourceGenerator> closest = nullptr;

    BuildTypes targetType = BuildTypes::Tree;

    // Hedef türünü mevcut kargo tipine göre belirle
    if (cargoType == ResourceType::Gold) targetType = BuildTypes::Gold; // <-- YENÝ: Öncelik yerdeki altýna verilebilir veya GoldMine ile birleþtirilebilir.
    // DÝKKAT: Eðer hem bina maden (GoldMine) hem yerdeki altýn (Gold) varsa, buraya bir mantýk eklemek gerekebilir.
    // Þimdilik yerdeki altýna odaklansýn:

    // Basit bir if/else yapýsý yerine, ne topluyorsak onun tipini arayalým:
    if (cargoType == ResourceType::Food) targetType = BuildTypes::Farm;
    else if (cargoType == ResourceType::Stone) targetType = BuildTypes::Stone;

    // Altýn için özel durum: Hem GoldMine hem Gold olabilir.
    // Þimdilik basitçe Gold (Yerdeki) arayalým.

    else if (cargoType == ResourceType::Gold) targetType = BuildTypes::Gold;

    for (const auto& building : buildings) {
        if (building && building->getIsAlive() && building->buildingType == targetType) {
            if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {

                if (res->isFull()) continue;

                sf::Vector2f diff = res->getPosition() - getPosition();
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                if (dist < minDistance) {
                    minDistance = dist;
                    closest = res;
                }
            }
        }
    }

    if (closest) {
        startHarvesting(closest);
    }
    else {
        stopHarvesting();
    }
}

std::string Villager::getName() {
    return "Villager #" + std::to_string(entityID);
}

void Villager::render(sf::RenderWindow& window) {
    if (state == VillagerState::Garrisoned) return;
    Unit::render(window);
}