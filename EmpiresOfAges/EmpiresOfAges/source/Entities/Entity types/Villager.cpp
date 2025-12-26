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
    // Görsel çok büyükse küçültmek için (Unit boyutu 20x20 gibi düþünülürse):
    float scaleX = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().x;
    float scaleY = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().y;
    this->setScale(scaleX, scaleY);
    // ------------------------------------------

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

    // Eðer hedeflediðimiz kaynak doluysa ve içinde biz yoksak, gitme.
    // Ancak oyuncu sað týkladýysa zorla gitmeye çalýþsýn, belki o gidene kadar boþalýr.
    targetResource = resource;
    state = VillagerState::MovingToResource;

    // Kaynak tipini belirle
    if (resource->buildingType == BuildTypes::Tree) cargoType = ResourceType::Wood;
    else if (resource->buildingType == BuildTypes::GoldMine) cargoType = ResourceType::Gold;
    else if (resource->buildingType == BuildTypes::StoneMine) cargoType = ResourceType::Stone;
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
        // Eðer bina yýkýldýysa veya kaynak tükendiyse dýþarý çýk
        if (!res || !res->getIsAlive()) {
            state = VillagerState::Idle;
            // Görünür olunca hafif kaydýr ki içine doðmasýn
            this->setPosition(this->getPosition() + sf::Vector2f(0, 50));
        }
        return; // Ýçerideyken baþka iþlem yapma
    }

    // 1. Durum: KAYNAÐA GÝDÝYOR
    if (state == VillagerState::MovingToResource) {
        auto res = targetResource.lock();

        // Kaynak yok olduysa hemen yenisini ara
        if (!res || !res->getIsAlive()) {
            findNearestResource(buildings);
            return;
        }

        sf::Vector2f diff = res->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        // --- DÜZELTME 1: MENZÝLÝ GENÝÞLETÝYORUZ ---
        // Askerlerin çarpýþma yarýçapý + Bina yarýçapý + Güvenlik payý
        // Bu sayede duvara çarpsa bile "vardým" sayacak.
        float targetRadius = res->getBounds().width / 2.0f;
        if (targetRadius < 32.0f) targetRadius = 32.0f; // Minimum (Aðaç için)

        float interactionDist = targetRadius + 15.0f; // 45px pay veriyoruz (Çarpýþma hatasýný önler)

        if (dist <= interactionDist) {

            // --- DÜZELTME 2: ÇÝFTLÝK VE NORMAL KAYNAK AYRIMI ---
            if (res->buildingType == BuildTypes::Farm) {
                if (res->garrisonWorker()) {
                    state = VillagerState::Garrisoned; // Ýçeri gir
                    m_path.clear();
                    m_isMoving = false;
                    isSelected = false;
                }
                else {
                    // Doluysa bekleme yapma, etrafta baþka çiftlik varsa ona bak
                    // Þimdilik duruyor, istenirse findNearestResource çaðrýlabilir.
                    stopHarvesting();
                }
            }
            // --- NORMAL KAYNAK (AÐAÇ/MADEN) ---
            else {
                if (res->garrisonWorker()) {
                    state = VillagerState::Harvesting;
                    m_path.clear();
                    m_isMoving = false;
                }
                else {
                    // --- DÜZELTME 3: AÐAÇ DOLUYSA BAÞKASINI ARA ---
                    // Eðer kapýsýna geldiðimiz aðaç doluysa, mal gibi bekleme, yanýndakine git.
                    findNearestResource(buildings);
                }
            }
        }
    }

    // 2. Durum: KAYNAK TOPLUYOR (Harvesting)
    else if (state == VillagerState::Harvesting) {
        auto res = targetResource.lock();

        // Kaynak bittiyse ne yapayým?
        if (!res || !res->getIsAlive()) {
            // --- DÜZELTME 4: ELÝM BOÞSA EVE DÖNME ---
            if (currentCargo > 0) {
                state = VillagerState::ReturningToBase;
            }
            else {
                findNearestResource(buildings); // Elim boþ, hemen yeni aðaç bul
            }
            return;
        }

        // Kaynaktan veri çek
        int amount = res->updateGeneration(dt);
        if (amount > 0) {
            currentCargo += amount;

            // Çanta doldu mu?
            if (currentCargo >= maxCargo) {
                currentCargo = maxCargo;
                res->releaseWorker();    // Kaynaktan çýk

                // Depo bul
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

        // Hedef depo yýkýldýysa yenisini bul
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

        // --- MENZÝL DÜZELTMESÝ ---
        float buildingRadius = base->getBounds().width / 2.0f;
        if (buildingRadius < 32.0f) buildingRadius = 32.0f;
        float interactionRange = buildingRadius + 50.0f; // Geniþ tolerans

        if (dist <= interactionRange) {
            // Boþalt
            if (cargoType == ResourceType::Wood) player.addWood(currentCargo);
            else if (cargoType == ResourceType::Food) player.addFood(currentCargo);
            else if (cargoType == ResourceType::Gold) player.addGold(currentCargo);
            else if (cargoType == ResourceType::Stone) player.addStone(currentCargo);

            currentCargo = 0;

            // Tekrar Kaynaða Dön
            auto res = targetResource.lock();
            // Eski kaynak duruyor mu ve DOLU DEÐÝL MÝ?
            if (res && res->getIsAlive() && !res->isFull()) {
                state = VillagerState::MovingToResource;
                moveTo(res->getPosition());
            }
            else {
                // Eski kaynak bittiyse veya baþkasý kaptýysa yenisini ara
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
    if (cargoType == ResourceType::Gold) targetType = BuildTypes::GoldMine;
    else if (cargoType == ResourceType::Food) targetType = BuildTypes::Farm;

    for (const auto& building : buildings) {
        if (building && building->getIsAlive() && building->buildingType == targetType) {
            if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {

                // --- KRÝTÝK: ZATEN DOLU OLANLARI HEDEFLEME ---
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
        // Yakýnda kaynak yoksa dur
        stopHarvesting();
    }
}

std::string Villager::getName() {
    // ID'yi ismin yanýna ekliyoruz: "Villager #1", "Villager #2" gibi
    return "Villager #" + std::to_string(entityID);
}

void Villager::render(sf::RenderWindow& window) {
    if (state == VillagerState::Garrisoned) return;
    Unit::render(window);
}