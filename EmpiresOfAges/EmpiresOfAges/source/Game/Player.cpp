#include "Game/Player.h"
#include "Entity System/Entity Type/Unit.h"     
#include "Entity System/Entity Type/Building.h"
#include <iostream>
#include <algorithm>

Player::Player() {
    // Starting Resources
    playerResources.add(ResourceType::Wood, 100);
    playerResources.add(ResourceType::Food, 100);
    playerResources.add(ResourceType::Gold, 0);
    playerResources.add(ResourceType::Stone, 50);

    unitLimit = 5;
    buildLimit = 999;
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

std::vector<std::shared_ptr<Entity>> Player::getEntities() {
    return entities;
}

void Player::renderEntities(sf::RenderWindow& window) {
    for (auto& entity : entities) {
        if (entity->getIsAlive()) {
            // Buildings are drawn by MapManager usually, but selection rings handled here
            if (std::dynamic_pointer_cast<Building>(entity)) {
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

    selected_entities.erase(
        std::remove_if(
            selected_entities.begin(),
            selected_entities.end(),
            [](const std::shared_ptr<Entity>& e) { return !e->getIsAlive(); }
        ),
        selected_entities.end()
    );
}

int Player::getUnitLimit() {
    return unitLimit;
}

int Player::getUnitCount() {
    int count = 0;
    for (auto& e : entities) {
        if (e->getIsAlive()) {
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

// --- SELECTION LOGIC ---

void Player::selectUnit(sf::RenderWindow& window, const sf::View& camera, bool isShiftHeld) {
    sf::Vector2i mousePosPixel = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosWorld = window.mapPixelToCoords(mousePosPixel, camera);

    bool clickedOnUnit = false;
    bool clickedOnBuilding = false;

    // 1. Check Units First
    for (auto& entity : entities) {
        if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                if (!isShiftHeld && !clickedOnUnit) {
                    for (auto& e : selected_entities) e->setSelected(false);
                    selected_entities.clear();
                }

                if (isShiftHeld && unit->isSelected) {
                    unit->setSelected(false);
                }
                else if (!unit->isSelected) {
                    unit->setSelected(true);
                    selected_entities.push_back(unit);
                }

                clickedOnUnit = true;
                break;
            }
        }
    }

    // 2. Check Buildings
    if (!clickedOnUnit) {
        for (auto& entity : entities) {
            if (entity->getIsAlive() && entity->getBounds().contains(mousePosWorld)) {
                if (auto building = std::dynamic_pointer_cast<Building>(entity)) {
                    for (auto& e : selected_entities) e->setSelected(false);
                    selected_entities.clear();

                    building->setSelected(true);
                    selected_entities.push_back(building);

                    clickedOnBuilding = true;
                    break;
                }
            }
        }
    }

    // 3. Clear Selection if clicked on nothing
    if (!clickedOnUnit && !clickedOnBuilding && !isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }
}

void Player::selectUnitsInRect(const sf::FloatRect& selectionRect, bool isShiftHeld) {
    if (!isShiftHeld) {
        for (auto& e : selected_entities) e->setSelected(false);
        selected_entities.clear();
    }

    for (auto& entity : entities) {
        if (!entity->getIsAlive()) continue;

        // Ignore buildings in box selection
        if (std::dynamic_pointer_cast<Building>(entity)) {
            continue;
        }

        if (selectionRect.intersects(entity->getBounds())) {
            if (!entity->isSelected) {
                entity->setSelected(true);
                selected_entities.push_back(entity);
            }
        }
    }
}