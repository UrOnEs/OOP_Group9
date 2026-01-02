#pragma once
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Entity System/Entity Type/types.h"
#include "Game/Player.h"

class MapManager;

enum class ProductionResult {
    Success,
    InsufficientFood,
    InsufficientWood,
    InsufficientGold,
    InsufficientStone,
    InsufficentResource,
    PopulationFull,
    QueueFull,
    InvalidBuilding
};

/**
 * @brief Handles the creation of units from buildings (Barracks, TownCenter).
 * Manages costs, queues, and spawning logic.
 */
class ProductionSystem {
public:
    // --- Soldier Production ---
    static ProductionResult startProduction(Player& player, Barracks& barracks, SoldierTypes unitType);
    static void update(Player& player, Barracks& barracks, float dt, MapManager& mapManager);

    // --- Villager Production ---
    static ProductionResult startVillagerProduction(Player& player, TownCenter& tc);
    static void updateTC(Player& player, TownCenter& tc, float dt, MapManager& mapManager);
};