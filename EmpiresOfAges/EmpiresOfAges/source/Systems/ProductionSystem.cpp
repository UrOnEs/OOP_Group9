#include "Systems/ProductionSystem.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/Soldier.h"
#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Map/MapManager.h" 
#include "Map/PathFinder.h"
#include <iostream>
#include <set>
//For warnings
#include "Game/Game.h"

ProductionResult ProductionSystem::startProduction(Player& player, Barracks& barracks, SoldierTypes unitType) {
    if (player.getCurrentPopulation() + 1 > player.getUnitLimit()) {
        std::cout << "[Production] Nufus limiti dolu!\n";
        return ProductionResult::PopulationFull;
    }
    // Maliyet Kontrolü
    GameRules::Cost cost = GameRules::getUnitCost(unitType);
    std::vector<int> res = player.getResources();

    if (res[0] < cost.wood) return ProductionResult::InsufficientWood;
    if (res[1] < cost.gold) return ProductionResult::InsufficientGold;
    if (res[2] < cost.stone) return ProductionResult::InsufficientStone;
    if (res[3] < cost.food) return ProductionResult::InsufficientFood;

    player.addWood(-cost.wood);
    player.addGold(-cost.gold);
    player.addStone(-cost.stone);
    player.addFood(-cost.food);

    player.addQueuedUnit(1);

    float time = 10.0f;
    if (unitType == SoldierTypes::Barbarian) time = 5.0f;

    barracks.startTraining(unitType, time);
    return ProductionResult::Success;
}

// ======================================================================================
//                                  ASKER GÜNCELLEME VE SPAWN
// ======================================================================================
void ProductionSystem::update(Player& player, Barracks& barracks, float dt, MapManager& mapManager) {
    if (!barracks.getIsProducing()) return;

    float timeMultiplier = GameRules::DebugMode ? 100.0f : 1.0f;

    barracks.updateTimer(dt * timeMultiplier);


    if (barracks.isReady()) {
        SoldierTypes type = barracks.finishTraining();

        std::shared_ptr<Soldier> newSoldier = std::make_shared<Soldier>();
        newSoldier->setType(type);
        newSoldier->setTeam(player.getTeamColor());

        // --- AKILLI SPAWN SİSTEMİ (DÜZELTİLDİ) ---
        Point buildingGrid = barracks.getGridPoint();

        // 1. REZERVE LİSTESİNİ DOLDUR
        // Sadece duvarlara değil, haritadaki DİĞER ASKERLERE de bakmalıyız.
        std::set<Point> reserved;
        for (const auto& entity : player.getEntities()) {
            if (entity->getIsAlive()) {
                reserved.insert(entity->getGridPoint());
            }
        }

        // 2. En yakın boş kareyi bul (Artık askerlerin olduğu kareleri de dolu sayacak)
        Point spawnGrid = PathFinder::findClosestFreeTile(
            buildingGrid, mapManager.getLevelData(), mapManager.getWidth(), mapManager.getHeight(), reserved
        );

        // 3. Dünya koordinatına çevir ve yerleştir
        float spawnX = spawnGrid.x * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
        float spawnY = spawnGrid.y * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;

        newSoldier->setPosition(sf::Vector2f(spawnX, spawnY));
        // -----------------------------------------

        player.addEntity(newSoldier);
        player.addQueuedUnit(-1);
        std::cout << "[INFO] Asker egitimi tamamlandi! (" << spawnGrid.x << "," << spawnGrid.y << ")\n";
    }
}

ProductionResult ProductionSystem::startVillagerProduction(Player& player, TownCenter& tc) {
  
    if (!tc.isConstructed) {
    std::cout << "[Production] Bina inşaat Ediliyor!\n";
    return ProductionResult::PopulationFull; // BURAYA YENİ BİR UYARI LAZIM !
    }
    if (player.getCurrentPopulation() + 1 > player.getUnitLimit()) {
        std::cout << "[Production] Nufus limiti dolu!\n";
        return ProductionResult::PopulationFull;
    }
    // Köylü Maliyeti
    int foodCost = 50;
    if (player.getResources()[3] >= foodCost) {
        player.addFood(-foodCost);
        player.addQueuedUnit(1);
        tc.startProduction(10.0f); // 10 saniye üretim
        return ProductionResult::Success;
    }
    return ProductionResult::InsufficientFood;
}

// ======================================================================================
//                                  KÖYLÜ GÜNCELLEME VE SPAWN
// ======================================================================================
void ProductionSystem::updateTC(Player& player, TownCenter& tc, float dt, MapManager& mapManager) {
    
    float timeMultiplier = GameRules::DebugMode ? 100.0f : 1.0f;

    tc.updateTimer(dt * timeMultiplier);

    if (tc.isReady()) {
        tc.finishProduction();

        std::shared_ptr<Villager> newVillager = std::make_shared<Villager>();
        newVillager->setTeam(player.getTeamColor());

        // --- AKILLI SPAWN SİSTEMİ (DÜZELTİLDİ) ---
        Point buildingGrid = tc.getGridPoint();

        // 1. REZERVE LİSTESİNİ DOLDUR
        std::set<Point> reserved;
        for (const auto& entity : player.getEntities()) {
            if (entity->getIsAlive()) {
                reserved.insert(entity->getGridPoint());
            }
        }

        // 2. En yakın boş kareyi bul
        Point spawnGrid = PathFinder::findClosestFreeTile(
            buildingGrid, mapManager.getLevelData(), mapManager.getWidth(), mapManager.getHeight(), reserved
        );

        float spawnX = spawnGrid.x * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
        float spawnY = spawnGrid.y * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;

        newVillager->setPosition(sf::Vector2f(spawnX, spawnY));
        // -----------------------------------------

        player.addEntity(newVillager);
        player.addQueuedUnit(-1);
        std::cout << "[INFO] Yeni koylu isbasi yapti! (" << spawnGrid.x << "," << spawnGrid.y << ")\n";
    }
}