#pragma once
#include "Building.h"

class TownCenter : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f;

    int queuedVillagers = 0;

public:
    TownCenter() {
        buildingType = BuildTypes::TownCenter;
        health = GameRules::HP_TownCenter;
        this->sprite.setOrigin(96.f, 150.f);
        addAbility(Ability(10, "Create Villager", "50 Food", "Gatherer", &AssetManager::getTexture("assets/units/default.png")));
    }

    int getMaxHealth() const override { return (int)GameRules::HP_TownCenter; }

    std::vector<sf::Texture*> getProductionQueueIcons() override {
        std::vector<sf::Texture*> icons;
        sf::Texture* villagerTex = &AssetManager::getTexture("assets/units/default.png");

        if (isProducing) icons.push_back(villagerTex);
        for (int i = 0; i < queuedVillagers; ++i) icons.push_back(villagerTex);
        return icons;
    }

    float getProductionProgress() override {
        if (!isProducing || productionDuration <= 0) return 0.0f;
        float progress = 1.0f - (productionTimer / productionDuration);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        return progress;
    }

    void startProduction(float duration) {
        if (!isConstructed) return;

        if (isProducing) {
            queuedVillagers++;
        }
        else {
            productionDuration = duration;
            productionTimer = productionDuration;
            isProducing = true;
        }
    }

    bool getIsProducing() const { return isProducing; }

    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    bool isReady() const { return isProducing && productionTimer <= 0; }

    void finishProduction() {
        if (queuedVillagers > 0) {
            queuedVillagers--;
            productionDuration = GameRules::Time_Build_Villager;
            productionTimer = productionDuration;
            isProducing = true;
        }
        else {
            isProducing = false;
        }
    }

    std::string getInfo() override { return "Town Center"; }

    void render(sf::RenderWindow& window) override {
        // Selection circle logic is handled, but specific override here for Town Center if needed
        // Reusing base selection render logic manually as per previous pattern
        if (isSelected) {
            float radius = 32.0f;
            if (hasTexture) {
                sf::FloatRect bounds = sprite.getGlobalBounds();
                radius = (bounds.width / 2.0f) * 1.1f;
            }
            sf::CircleShape selectionCircle(radius);
            selectionCircle.setPointCount(40);
            selectionCircle.setOrigin(radius, radius - 60);
            selectionCircle.setPosition(getPosition());
            selectionCircle.setScale(1.0f, 0.6f);
            selectionCircle.setFillColor(sf::Color(0, 255, 0, 50));
            selectionCircle.setOutlineColor(sf::Color::Green);
            selectionCircle.setOutlineThickness(3.0f);
            window.draw(selectionCircle);
        }

        Building::render(window);

        // Production Bar
        if (isProducing) {
            float barWidth = 100.0f;
            float barHeight = 10.0f;

            sf::Vector2f pos = getPosition();
            float x = pos.x - barWidth / 2.0f;
            float y = pos.y - 120.0f;

            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setPosition(x, y);
            backBar.setFillColor(sf::Color(50, 50, 50));
            backBar.setOutlineThickness(1);
            backBar.setOutlineColor(sf::Color::Black);

            float percent = 1.0f - (productionTimer / productionDuration);
            if (percent < 0) percent = 0; if (percent > 1) percent = 1;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * percent, barHeight));
            frontBar.setPosition(x, y);
            frontBar.setFillColor(sf::Color(0, 191, 255));

            window.draw(backBar);
            window.draw(frontBar);
            Building::render(window);
        }
    }
};