#include "Systems/BuildSystem.h"
#include "Game/Player.h"
#include "Game/GameRules.h"
#include "Map/MapManager.h"

bool BuildSystem::build(Player& player, Villager& worker, Building& building, const sf::Vector2f& pos) {
    // 1. Get Building Cost
    GameRules::Cost cost = GameRules::getBuildingCost(building.buildingType);

    // 2. Check Player Resources
    // Accessing resources via a temporary vector copy (assuming Player has getResources)
    std::vector<int> currentRes = player.getResources();
    if (currentRes[0] < cost.wood || currentRes[1] < cost.gold) {
        return false; // Insufficient resources
    }

    // 3. Check Space Availability
    if (!checkSpace(building, pos)) {
        return false;
    }

    // 4. Execute Construction
    // Deduct resources
    player.addWood(-cost.wood);
    player.addGold(-cost.gold);
    player.addFood(-cost.food);
    player.addStone(-cost.stone);

    building.setPosition(pos);
    return true;
}

bool BuildSystem::checkSpace(Building& building, const sf::Vector2f& pos) {
    // Logic to be implemented via MapManager to check for collisions/obstacles.
    return true;
}