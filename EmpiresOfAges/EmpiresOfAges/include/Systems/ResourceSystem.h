#pragma once
#include "Game/Player.h"
#include "Entity System/Entity Type/ResourceGenerator.h"

class ResourceSystem {
public:
    static void update(Player& player, ResourceGenerator& building, float dt) {

        int producedAmount = building.updateGeneration(dt);

        if (producedAmount > 0) {
            // Hangi bina olduðuna göre doðru kaynaðý ekle
            if (building.buildingType == BuildTypes::Farm) {
                player.playerResources.add(ResourceType::Food,producedAmount); // Player'a addFood eklemediysen ekle
                // Efekt/Ses çalýnabilir
            }
            else if (building.buildingType == BuildTypes::StoneMine) {
                player.playerResources.add(ResourceType::Stone, producedAmount);
            }
            // --- YENÝ EKLENEN KISIM ---
            else if (building.buildingType == BuildTypes::Tree) {
                player.playerResources.add(ResourceType::Wood, producedAmount);
            }
        }
    }
};
