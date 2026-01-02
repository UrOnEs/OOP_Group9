#pragma once
#include "Building.h"
#include <iostream>

class Mountain : public Building {
public:
    Mountain() {
        buildingType = BuildTypes::Mountain;
        isConstructed = true;
        health = 100000;
    }

    std::string getInfo() override { return "Mountain"; }
    int getMaxHealth() const override { return 100000; }

    // Autotiling logic
    void setVariation(int mask, int tileSize) {
        if (!hasTexture) return;

        int col = mask % 4;
        int row = mask / 4;

        sf::IntRect rect(col * tileSize, row * tileSize, tileSize, tileSize);
        sprite.setTextureRect(rect);
        sprite.setOrigin(tileSize / 2.0f, tileSize / 2.0f);
    }
};