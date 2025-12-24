#pragma once
#include "Entity System/Entity.h"
#include "types.h"
#include "Game/GameRules.h" // GameRules eklendi

class Building : public Entity {
public:
    BuildTypes buildingType;

    Building() {
        // health = 500.f;  <-- BUNU SÝLÝYORUZ. 
        // Artýk varsayýlan can yok, alt sýnýflar atayacak.
        // Ama güvenlik için yine de bir baþlangýç deðeri verelim:
        health = GameRules::BuildingHealth;
    }

    virtual ~Building() = default;

    std::string stats() override {
        return getInfo();
    }

    virtual std::string getInfo() = 0;


    std::string getName() override {
        switch (buildingType) {
        case BuildTypes::Barrack:    return "Barracks";
        case BuildTypes::Farm:       return "Farm";
        case BuildTypes::WoodPlace:  return "Lumber Camp"; // Veya WoodPlace
        case BuildTypes::StoneMine:  return "Stone Mine";
        case BuildTypes::GoldMine:   return "Gold Mine";
        case BuildTypes::House:      return "House";
        case BuildTypes::TownCenter: return "Town Center";
        default:                     return "Building";
        }
    }

    // Ayrýca getIcon için basit bir yönlendirme yapabiliriz
    sf::Texture* getIcon() override {
        if (hasTexture) return (sf::Texture*)sprite.getTexture();
        return nullptr;
    }

    // ...
};