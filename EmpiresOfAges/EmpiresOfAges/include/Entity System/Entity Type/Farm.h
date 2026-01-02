#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class Farm : public ResourceGenerator {
public:
    Farm() {
        buildingType = BuildTypes::Farm;
        health = GameRules::HP_Farm;
        interval = 2.0f;
        amountPerTick = 10;
        maxWorkers = 4;
    }

    int getMaxHealth() const override { return (int)GameRules::HP_Farm; }
    std::string getInfo() override { return "Farm: Food Source (Max 4 workers)"; }

    void render(sf::RenderWindow& window) override {
        if (isSelected) {
            float radius = 32.0f;
            if (hasTexture) {
                sf::FloatRect bounds = sprite.getGlobalBounds();
                radius = (bounds.width / 2.0f) * 1.1f;
            }
            sf::CircleShape selectionCircle(radius);
            selectionCircle.setPointCount(40);
            selectionCircle.setOrigin(radius, radius - 35);
            selectionCircle.setPosition(getPosition());
            selectionCircle.setScale(1.0f, 0.6f);
            selectionCircle.setFillColor(sf::Color(0, 255, 0, 50));
            selectionCircle.setOutlineColor(sf::Color::Green);
            selectionCircle.setOutlineThickness(3.0f);
            window.draw(selectionCircle);
        }
        Building::render(window);
    }
};