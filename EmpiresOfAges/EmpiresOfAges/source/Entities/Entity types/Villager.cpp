#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/Building.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/ResourceGenerator.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <iostream>
#include <cmath>
#include <algorithm> 
#include <set>

// Helper: Calculate shortest distance to a rectangle
float getDistanceToRect(const sf::Vector2f& p, const sf::FloatRect& r) {
    float dx = std::max({ r.left - p.x, 0.f, p.x - (r.left + r.width) });
    float dy = std::max({ r.top - p.y, 0.f, p.y - (r.top + r.height) });
    return std::sqrt(dx * dx + dy * dy);
}

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
    this->maxCargo = GameRules::Villager_MaxCargo;
    this->setTexture(AssetManager::getTexture("assets/units/default.png"));

    if (this->sprite.getTexture()) {
        float scaleX = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().x;
        float scaleY = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().y;
        this->setScale(scaleX, scaleY);
    }

    addAbility(Ability(1, "Build House", "30 Wood", "+5 Pop", &AssetManager::getTexture("assets/buildings/house.png")));
    addAbility(Ability(2, "Build Barracks", "175 Wood", "Train Troops", &AssetManager::getTexture("assets/buildings/barrack.png")));
    addAbility(Ability(3, "Build Farm", "60 Wood", "Food Production", &AssetManager::getTexture("assets/buildings/mill.png")));
    addAbility(Ability(4, "Build TC", "400 Wood, 200 Stone", "Main Base", &AssetManager::getTexture("assets/buildings/castle.png")));

    counter++;
}

Villager::~Villager() { counter--; }

std::string Villager::stats() {
    return "Load: " + std::to_string(currentCargo) + "/" + std::to_string(maxCargo);
}

int Villager::getMaxHealth() const { return GameRules::HP_Villager; }

void Villager::startHarvesting(std::shared_ptr<ResourceGenerator> resource) {
    if (!resource) return;

    targetResource = resource;
    state = VillagerState::MovingToResource;

    if (resource->buildingType == BuildTypes::Tree) cargoType = ResourceType::Wood;
    else if (resource->buildingType == BuildTypes::GoldMine) cargoType = ResourceType::Gold;
    else if (resource->buildingType == BuildTypes::StoneMine) cargoType = ResourceType::Stone;
    else if (resource->buildingType == BuildTypes::Stone) cargoType = ResourceType::Stone;
    else if (resource->buildingType == BuildTypes::Gold) cargoType = ResourceType::Gold;
    else if (resource->buildingType == BuildTypes::Farm) cargoType = ResourceType::Food;

    m_path.clear();
    m_isMoving = false;
}

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

void Villager::smartMoveTo(sf::Vector2f targetPos, int targetSizeInTiles, const std::vector<int>& mapData, int width, int height) {
    Point targetGrid = { (int)(targetPos.x / GameRules::TileSize), (int)(targetPos.y / GameRules::TileSize) };
    Point myGrid = getGridPoint();
    std::set<Point> reserved;

    // Find best accessible tile around target
    Point safeTarget = PathFinder::findBestTargetTile(myGrid, targetGrid, targetSizeInTiles, mapData, width, height, reserved);

    std::vector<Point> pathPoints = PathFinder::findPath(myGrid, safeTarget, mapData, width, height);

    if (!pathPoints.empty()) {
        std::vector<sf::Vector2f> worldPath;
        for (auto& p : pathPoints) {
            float px = p.x * GameRules::TileSize + GameRules::TileSize / 2.0f;
            float py = p.y * GameRules::TileSize + GameRules::TileSize / 2.0f;
            worldPath.push_back(sf::Vector2f(px, py));
        }
        setPath(worldPath);
    }
    else {
        m_isMoving = false;
    }
}

void Villager::updateVillager(float dt, const std::vector<std::shared_ptr<Building>>& buildings, Player& player, const std::vector<int>& mapData, int width, int height) {

    // --- STATE: BUILDING ---
    if (state == VillagerState::Building) {
        auto building = targetConstruction.lock();

        if (!building || !building->getIsAlive() || building->isConstructed) {
            state = VillagerState::Idle;
            return;
        }

        sf::Vector2f diff = building->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        float workDist = (building->getBounds().width / 2.0f) + 20.0f;

        if (dist <= workDist) {
            m_path.clear(); m_isMoving = false;
            float buildSpeed = GameRules::Villager_BuildSpeed;
            float buildAmount = buildSpeed * dt;

            if (building->health < building->getMaxHealth()) {
                building->health += buildAmount;
                if (GameRules::DebugMode) building->health += building->getMaxHealth();

                if (building->health >= building->getMaxHealth()) {
                    building->health = (float)building->getMaxHealth();

                    if (!building->isConstructed) {
                        building->isConstructed = true;
                        building->onConstructionComplete(player);
                    }
                    state = VillagerState::Idle;
                }
            }
        }
        else {
            if (!m_isMoving) moveTo(building->getPosition());
        }
    }

    // --- STATE: GARRISONED ---
    if (state == VillagerState::Garrisoned) {
        auto res = targetResource.lock();
        if (!res || !res->getIsAlive()) {
            state = VillagerState::Idle;
            this->setPosition(this->getPosition() + sf::Vector2f(0, 40));
        }
        return;
    }

    // --- STATE: MOVING TO RESOURCE ---
    if (state == VillagerState::MovingToResource) {
        auto res = targetResource.lock();
        if (!res || !res->getIsAlive()) { findNearestResource(buildings); return; }

        if (!m_isMoving && m_path.empty()) {
            int size = 2;
            if (res->buildingType == BuildTypes::TownCenter) size = 6;
            else if (res->buildingType == BuildTypes::Farm) size = 4;
            else if (res->buildingType == BuildTypes::Tree) size = 2;

            smartMoveTo(res->getPosition(), size, mapData, width, height);
        }

        float distToEdge = getDistanceToRect(getPosition(), res->getBounds());

        if (distToEdge <= 40.0f) {
            if (res->buildingType == BuildTypes::Farm) {
                if (res->garrisonWorker()) {
                    state = VillagerState::Garrisoned;
                    m_path.clear(); m_isMoving = false;
                    isSelected = false;
                }
                else stopHarvesting();
            }
            else {
                if (res->garrisonWorker()) {
                    state = VillagerState::Harvesting;
                    m_path.clear(); m_isMoving = false;
                }
                else findNearestResource(buildings);
            }
        }
        else {
            if (!m_isMoving) {
                int size = 2;
                if (res->buildingType == BuildTypes::TownCenter) size = 6;
                else if (res->buildingType == BuildTypes::Farm) size = 4;
                smartMoveTo(res->getPosition(), size, mapData, width, height);
            }
        }
    }

    // --- STATE: HARVESTING ---
    else if (state == VillagerState::Harvesting) {
        auto res = targetResource.lock();
        if (!res || !res->getIsAlive()) {
            if (currentCargo > 0) state = VillagerState::ReturningToBase;
            else findNearestResource(buildings);
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
                    m_path.clear(); m_isMoving = false;
                }
                else {
                    stopHarvesting();
                }
            }
        }
    }

    // --- STATE: RETURNING TO BASE ---
    else if (state == VillagerState::ReturningToBase) {
        auto base = targetBase.lock();
        if (!base || !base->getIsAlive()) {
            base = findNearestBase(buildings);
            if (base) {
                targetBase = base;
                m_path.clear(); m_isMoving = false;
            }
            else return;
        }

        if (!m_isMoving && m_path.empty()) {
            int size = 6;
            if (base->buildingType != BuildTypes::TownCenter) size = 2;
            smartMoveTo(base->getPosition(), size, mapData, width, height);
        }

        float distToEdge = getDistanceToRect(getPosition(), base->getBounds());

        if (distToEdge <= 40.0f) {
            if (cargoType == ResourceType::Wood) player.addWood(currentCargo);
            else if (cargoType == ResourceType::Food) player.addFood(currentCargo);
            else if (cargoType == ResourceType::Gold) player.addGold(currentCargo);
            else if (cargoType == ResourceType::Stone) player.addStone(currentCargo);

            currentCargo = 0;

            auto res = targetResource.lock();
            if (res && res->getIsAlive() && !res->isFull()) {
                state = VillagerState::MovingToResource;
                m_path.clear(); m_isMoving = false;
            }
            else {
                findNearestResource(buildings);
            }
        }
        else {
            if (!m_isMoving) {
                int size = 6;
                if (base->buildingType != BuildTypes::TownCenter) size = 2;
                smartMoveTo(base->getPosition(), size, mapData, width, height);
            }
        }
    }
}

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

void Villager::findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings) {
    float minDistance = 2000.0f;
    std::shared_ptr<ResourceGenerator> closest = nullptr;

    BuildTypes targetType = BuildTypes::Tree;
    if (cargoType == ResourceType::Food) targetType = BuildTypes::Farm;
    else if (cargoType == ResourceType::Stone) targetType = BuildTypes::Stone;
    else if (cargoType == ResourceType::Gold) targetType = BuildTypes::Gold;

    for (const auto& building : buildings) {
        if (building && building->getIsAlive() && building->buildingType == targetType) {
            if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {
                if (res->isFull()) continue;

                float dist = getDistanceToRect(getPosition(), res->getBounds());

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

void Villager::startBuilding(std::shared_ptr<Building> building) {
    if (!building) return;
    targetConstruction = building;
    state = VillagerState::Building;
}