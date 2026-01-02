#pragma once
#include "Entity System/Entity.h"
#include "types.h"
#include "Game/GameRules.h" 
#include <vector>            
#include "UI/AssetManager.h" 

/**
 * @brief Base class for all static structures in the game.
 * * This class inherits from Entity and adds functionality specific to buildings,
 * such as construction states, production queues, and specific rendering logic
 * to handle the visual transition from construction to completion.
 */
class Building : public Entity {
public:
    BuildTypes buildingType;
    bool isConstructed = true;

    /**
     * @brief Constructor initializing default building health.
     */
    Building() {
        health = GameRules::BuildingHealth;
    }

    virtual ~Building() = default;

    /**
     * @brief Returns a string representation of the building's stats.
     * @return String containing info (e.g., name, health).
     */
    std::string stats() override {
        return getInfo();
    }

    /**
     * @brief Pure virtual function to get specific building information.
     */
    virtual std::string getInfo() = 0;

    /**
     * @brief Callback triggered when construction is finished.
     * @param owner The player who owns this building.
     */
    virtual void onConstructionComplete(class Player& owner) {}

    /**
     * @brief Retrieves the display name of the building based on its type.
     */
    std::string getName() override {
        switch (buildingType) {
        case BuildTypes::Barrack:    return "Barracks";
        case BuildTypes::Farm:       return "Farm";
        case BuildTypes::WoodPlace:  return "Lumber Camp";
        case BuildTypes::StoneMine:  return "Stone Mine";
        case BuildTypes::GoldMine:   return "Gold Mine";
        case BuildTypes::House:      return "House";
        case BuildTypes::TownCenter: return "Town Center";
        default:                     return "Building";
        }
    }

    /**
     * @brief Gets the icon texture for the UI.
     */
    sf::Texture* getIcon() override {
        if (hasTexture) return (sf::Texture*)sprite.getTexture();
        return nullptr;
    }

    // --- Production Queue Interface ---

    /**
     * @brief Retrieves icons for units currently in the production queue.
     */
    virtual std::vector<sf::Texture*> getProductionQueueIcons() {
        return {};
    }

    /**
     * @brief Gets the progress of the current production (0.0 to 1.0).
     */
    virtual float getProductionProgress() {
        return 0.0f;
    }

    /**
     * @brief Overrides the default render method to handle construction visuals.
     * * Unlike standard Entities, Buildings have a "construction" state where a scaffold
     * sprite is drawn instead of the building itself. This method manages:
     * 1. Drawing the selection circle (manually, as Entity::render is skipped).
     * 2. Drawing either the construction sprite OR the finished building sprite.
     * 3. Drawing the health bar if selected or damaged.
     * * @param window The render window target.
     */
    virtual void render(sf::RenderWindow& window) override {

        // 1. Draw Selection Circle
        // We handle this manually because we are bypassing Entity::render to avoid double drawing.
        if (isSelected) {
            float radius = 32.0f;
            if (hasTexture) {
                sf::FloatRect bounds = sprite.getGlobalBounds();
                // Calculate radius based on sprite width with a small padding
                radius = (bounds.width / 2.0f) * 1.1f;
            }

            sf::CircleShape selectionCircle(radius);
            selectionCircle.setPointCount(40);
            selectionCircle.setOrigin(radius, radius - 10);
            selectionCircle.setPosition(getPosition());

            // Apply isometric perspective effect
            selectionCircle.setScale(1.0f, 0.6f);

            selectionCircle.setFillColor(sf::Color(0, 255, 0, 50));
            selectionCircle.setOutlineColor(sf::Color::Green);
            selectionCircle.setOutlineThickness(3.0f);

            window.draw(selectionCircle);
        }

        // 2. Draw Visuals (Construction or Finished)
        if (!isConstructed) {
            // --- Construction State ---
            sf::Sprite constructionSprite;
            sf::Texture& tex = AssetManager::getTexture("assets/buildings/construction.png");

            if (tex.getSize().x > 0) {
                constructionSprite.setTexture(tex);

                // Scale the construction sprite to match the building's intended size
                sf::FloatRect buildingBounds = this->getBounds();
                sf::Vector2u texSize = tex.getSize();

                if (texSize.x > 0 && texSize.y > 0) {
                    float scaleX = buildingBounds.width / (float)texSize.x;
                    float scaleY = buildingBounds.height / (float)texSize.y;
                    constructionSprite.setScale(scaleX, scaleY);
                    constructionSprite.setOrigin(texSize.x / 2.0f, texSize.y / 2.0f);
                }
                constructionSprite.setPosition(getPosition());
                window.draw(constructionSprite);
            }
            else {
                // Fallback visual if texture is missing
                shape.setFillColor(sf::Color(100, 100, 100)); // Grey for unfinished
                window.draw(shape);
            }
        }
        else {
            // --- Finished State ---
            if (hasTexture) {
                window.draw(sprite);
            }
            else {
                window.draw(shape);
            }
        }

        // 3. Draw Health Bar
        // Only visible if the unit is alive AND (selected OR damaged)
        if (isAlive && (isSelected || health < getMaxHealth())) {
            float barWidth = 40.0f;
            float barHeight = 5.0f;

            sf::Vector2f pos = getPosition();
            // Position the bar above the sprite
            if (hasTexture) pos.y -= sprite.getGlobalBounds().height / 2.0f + 10.0f;
            else pos.y -= 25.0f;

            // Background (Red/Dark)
            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setOrigin(barWidth / 2, barHeight / 2);
            backBar.setPosition(pos);
            backBar.setFillColor(sf::Color(100, 0, 0));

            // Foreground (Health percentage)
            float hpPercent = (float)health / getMaxHealth();
            if (hpPercent < 0) hpPercent = 0;
            if (hpPercent > 1) hpPercent = 1;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * hpPercent, barHeight));
            frontBar.setOrigin(barWidth / 2, barHeight / 2);
            // Adjust X alignment to shrink towards the left
            frontBar.setPosition(pos.x - (barWidth * (1 - hpPercent)) / 2.0f, pos.y);

            // Color coding based on health status
            if (hpPercent > 0.5f) frontBar.setFillColor(sf::Color::Green);
            else if (hpPercent > 0.25f) frontBar.setFillColor(sf::Color::Yellow);
            else frontBar.setFillColor(sf::Color::Red);

            window.draw(backBar);
            window.draw(frontBar);
        }
    }
};