#pragma once
#include "Entity System/Entity.h"
#include "types.h"
#include "Game/GameRules.h" 
#include <vector>            
#include "UI/AssetManager.h" 

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

    // Üretim sýrasýndaki birimlerin ikonlarýný döndürür.
    // Ýlk eleman = Þu an üretilen. Diðerleri = Sýrada bekleyenler.
    virtual std::vector<sf::Texture*> getProductionQueueIcons() {
        return {}; // Varsayýlan boþ
    }

    // Þu anki üretimin ilerleme yüzdesi (0.0 ile 1.0 arasý)
    virtual float getProductionProgress() {
        return 0.0f;
    }
};