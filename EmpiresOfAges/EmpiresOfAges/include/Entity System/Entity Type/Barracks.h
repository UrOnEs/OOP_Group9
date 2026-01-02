#pragma once
#include "Building.h"
#include "Soldier.h"
#include <deque>

class Barracks : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f;
    SoldierTypes currentProduction;

    std::deque<SoldierTypes> productionQueue;

    sf::Texture* getUnitTexture(SoldierTypes type) {
        switch (type) {
        case SoldierTypes::Barbarian: return &AssetManager::getTexture("assets/units/barbarian.png");
        case SoldierTypes::Archer:    return &AssetManager::getTexture("assets/units/archer.png");
        case SoldierTypes::Wizard:    return &AssetManager::getTexture("assets/units/wizard.png");
        default: return nullptr;
        }
    }

public:
    Barracks() {
        buildingType = BuildTypes::Barrack;
        health = GameRules::HP_Barracks;
        this->sprite.setOrigin(64.f, 100.f);

        addAbility(Ability(11, "Train Barbarian", "60 Food, 20 Gold", "Basic Warrior", getUnitTexture(SoldierTypes::Barbarian)));
        addAbility(Ability(12, "Train Archer", "25 Wood, 20 Food, 45 Gold", "Ranged Unit", getUnitTexture(SoldierTypes::Archer)));
        addAbility(Ability(13, "Train Wizard", "160 Wood, 60 Food, 135 Gold", "AoE Mage", getUnitTexture(SoldierTypes::Wizard)));
    }

    int getMaxHealth() const override { return (int)GameRules::HP_Barracks; }

    std::vector<sf::Texture*> getProductionQueueIcons() override {
        std::vector<sf::Texture*> icons;
        if (isProducing) icons.push_back(getUnitTexture(currentProduction));
        for (const auto& type : productionQueue) icons.push_back(getUnitTexture(type));
        return icons;
    }

    float getProductionProgress() override {
        if (!isProducing || productionDuration <= 0) return 0.0f;
        float progress = 1.0f - (productionTimer / productionDuration);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        return progress;
    }

    void startTraining(SoldierTypes type, float duration) {
        if (!isConstructed) return;

        if (isProducing) {
            productionQueue.push_back(type);
        }
        else {
            currentProduction = type;
            productionTimer = duration;
            productionDuration = duration;
            isProducing = true;
        }
    }

    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    bool isReady() const { return isProducing && productionTimer <= 0; }

    SoldierTypes finishTraining() {
        SoldierTypes finishedUnit = currentProduction;

        if (!productionQueue.empty()) {
            currentProduction = productionQueue.front();
            productionQueue.pop_front();

            if (currentProduction == SoldierTypes::Barbarian) productionDuration = 5.0f;
            else if (currentProduction == SoldierTypes::Archer) productionDuration = 7.0f;
            else if (currentProduction == SoldierTypes::Wizard) productionDuration = 10.0f;
            else productionDuration = 10.0f;

            productionTimer = productionDuration;
            isProducing = true;
        }
        else {
            isProducing = false;
        }
        return finishedUnit;
    }

    bool getIsProducing() const { return isProducing; }
    std::string getInfo() override { return "Barracks"; }

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

        if (isProducing) {
            float barWidth = 80.0f;
            float barHeight = 8.0f;

            sf::Vector2f pos = getPosition();
            float x = pos.x - barWidth / 2.0f;
            float y = pos.y - 70.0f;

            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setPosition(x, y);
            backBar.setFillColor(sf::Color(50, 50, 50));
            backBar.setOutlineThickness(1);
            backBar.setOutlineColor(sf::Color::Black);

            float percent = 1.0f - (productionTimer / productionDuration);
            if (percent < 0) percent = 0; if (percent > 1) percent = 1;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * percent, barHeight));
            frontBar.setPosition(x, y);
            frontBar.setFillColor(sf::Color(255, 165, 0));

            window.draw(backBar);
            window.draw(frontBar);
            Building::render(window);
        }
    }
};