#pragma once
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h" // Castle
#include "Entity System/Entity Type/types.h"
#include "Game/Player.h"

class ProductionSystem {
public:
    // Asker Üretimi (Kýþla)
    static bool startProduction(Player& player, Barracks& barracks, SoldierTypes unitType);
    static void update(Player& player, Barracks& barracks, float dt);

    // --- YENÝ: Köylü Üretimi (Ana Bina) ---
    static bool startVillagerProduction(Player& player, TownCenter& tc);
    static void updateTC(Player& player, TownCenter& tc, float dt);
};