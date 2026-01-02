#pragma once

#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include "TeamColors.h"
#include "ResourceManager.h"

/**
 * @brief Represents a player in the game, managing entities, resources, and limits.
 */
class Player {
public:
    Player();
    ~Player();

    /**
     * @brief Selects a single unit under the mouse cursor.
     */
    void selectUnit(sf::RenderWindow& window, const sf::View& camera, bool isShiftHeld);

    /**
     * @brief Selects multiple units within a rectangular area.
     */
    void selectUnitsInRect(const sf::FloatRect& selectionRect, bool isShiftHeld);

    void renderEntities(sf::RenderWindow& window);
    void removeDeadEntities();

    // --- Resources & Entities ---
    std::vector<int> getResources();
    std::vector<std::shared_ptr<Entity>> getEntities();

    void addWood(int amount) { playerResources.add(ResourceType::Wood, amount); }
    void addGold(int amount) { playerResources.add(ResourceType::Gold, amount); }
    void addFood(int amount) { playerResources.add(ResourceType::Food, amount); }
    void addStone(int amount) { playerResources.add(ResourceType::Stone, amount); }

    void addEntity(std::shared_ptr<Entity> entity) {
        entities.push_back(entity);
    }

    // --- Limits & Population ---
    bool addUnitLimit(int amount);
    bool setUnitLimit(int amount);
    int getUnitLimit();
    int getUnitCount();

    void addQueuedUnit(int amount) {
        queuedUnits += amount;
    }

    int getCurrentPopulation() {
        return getUnitCount() + queuedUnits;
    }

    TeamColors getTeamColor() const { return Color; }
    void setTeamColor(TeamColors c) { Color = c; }

    void setName(const std::string& name) { m_name = name; }
    std::string getName() const { return m_name; }

    std::vector<std::shared_ptr<Entity>> entities;         ///< All owned entities
    std::vector<std::shared_ptr<Entity>> selected_entities;///< Currently selected entities
    ResourceManager playerResources;

private:
    std::string m_name = "Player";
    bool hasBase = true;
    TeamColors Color = TeamColors::Blue;

    int queuedUnits = 0;
    int unitLimit = 10;
    int buildLimit = 5;
};