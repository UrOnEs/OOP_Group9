#pragma once
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Entity System/Entity Type/types.h"
#include "Game/Player.h"

// MapManager sýnýfýný tanýmasý için
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

class ProductionSystem {
public:
    // Asker Üretimi
    static ProductionResult startProduction(Player& player, Barracks& barracks, SoldierTypes unitType);

    // --- GÜNCELLEME: MapManager eklendi ---
    static void update(Player& player, Barracks& barracks, float dt, MapManager& mapManager);

    // Köylü Üretimi
    static ProductionResult startVillagerProduction(Player& player, TownCenter& tc);

    // --- GÜNCELLEME: MapManager eklendi ---
    static void updateTC(Player& player, TownCenter& tc, float dt, MapManager& mapManager);
};