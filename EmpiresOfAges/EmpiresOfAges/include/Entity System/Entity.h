#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Game/TeamColors.h"
#include "UI/Ability.h"
#include "UI/AssetManager.h"
#include "Game/GameRules.h" 
#include "Map/Point.h"      

/**
 * @brief Base class for all interactive objects in the game.
 * Handles rendering, position, team ownership, and basic stats like health.
 */
class Entity {
protected:
    sf::Sprite sprite;
    bool hasTexture = false;
    std::vector<Ability> m_abilities;

    // Fallback shape if no texture is present
    sf::CircleShape shape;

    TeamColors m_team = TeamColors::Blue;

public:
    int entityID;
    float health;
    float damage;
    float range;
    bool isAlive;
    bool isSelected;

    Entity() {
        isAlive = true;
        isSelected = false;
        hasTexture = false;

        shape.setRadius(15.f);
        shape.setOrigin(15.f, 15.f);
        shape.setPointCount(30);
        shape.setFillColor(sf::Color::White);
    }

    virtual int getMaxHealth() const = 0;

    virtual ~Entity() = default;

    // --- Movement & Position ---
    void move(const sf::Vector2f& offset) {
        if (hasTexture) sprite.move(offset);
        shape.move(offset);
    }

    virtual void setPosition(const sf::Vector2f& newPos) {
        if (hasTexture) sprite.setPosition(newPos);
        shape.setPosition(newPos);
    }

    void setScale(float x, float y) {
        if (hasTexture) sprite.setScale(x, y);
        shape.setScale(x, y);
    }

    // --- UI Helpers ---
    virtual std::string getName() { return "Unknown"; }
    virtual sf::Texture* getIcon() { return nullptr; }

    void addAbility(const Ability& ability) {
        m_abilities.push_back(ability);
    }

    std::vector<Ability> getAbilities() const {
        return m_abilities;
    }

    virtual sf::Vector2f getPosition() const {
        return hasTexture ? sprite.getPosition() : shape.getPosition();
    }

    /**
     * @brief Returns the grid coordinates of the entity based on tile size.
     */
    Point getGridPoint() const {
        return {
            static_cast<int>(getPosition().x / GameRules::TileSize),
            static_cast<int>(getPosition().y / GameRules::TileSize)
        };
    }

    // --- Appearance ---
    void setTexture(const sf::Texture& texture) {
        sprite.setTexture(texture);
        sf::Vector2u size = texture.getSize();
        sprite.setOrigin(size.x / 2.f, size.y / 2.f);
        hasTexture = true;
    }

    void setTeam(TeamColors newTeam) {
        m_team = newTeam;

        sf::Color c = sf::Color::White;
        if (m_team == TeamColors::Red) c = sf::Color::Red;
        else if (m_team == TeamColors::Blue) c = sf::Color(0, 100, 255);
        else if (m_team == TeamColors::Green) c = sf::Color::Green;
        else if (m_team == TeamColors::Purple) c = sf::Color::Magenta;

        shape.setFillColor(c);
        if (hasTexture) sprite.setColor(c);
    }

    TeamColors getTeam() const { return m_team; }
    sf::CircleShape& getShape() { return shape; }
    float getRange() const { return range; }

    // --- Rendering ---
    virtual void render(sf::RenderWindow& window) {
        // 1. Draw Selection Circle
        if (isSelected) {
            float radius = 32.0f;

            if (hasTexture) {
                sf::FloatRect bounds = sprite.getGlobalBounds();
                radius = (bounds.width / 2.0f) * 1.1f;
            }

            sf::CircleShape selectionCircle(radius);
            selectionCircle.setPointCount(40);
            selectionCircle.setOrigin(radius, radius - 10);
            selectionCircle.setPosition(getPosition());

            // Isometric squish effect
            selectionCircle.setScale(1.0f, 0.6f);

            selectionCircle.setFillColor(sf::Color(0, 255, 0, 50));
            selectionCircle.setOutlineColor(sf::Color::Green);
            selectionCircle.setOutlineThickness(3.0f);

            window.draw(selectionCircle);
        }

        // 2. Draw Entity
        if (hasTexture) {
            window.draw(sprite);
        }
        else {
            window.draw(shape);
        }

        // 3. Draw Health Bar
        if (isAlive && (isSelected || health < getMaxHealth())) {
            float barWidth = 40.0f;
            float barHeight = 5.0f;

            sf::Vector2f pos = getPosition();
            if (hasTexture) pos.y -= sprite.getGlobalBounds().height / 2.0f + 10.0f;
            else pos.y -= 25.0f;

            // Background
            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setOrigin(barWidth / 2, barHeight / 2);
            backBar.setPosition(pos);
            backBar.setFillColor(sf::Color(100, 0, 0));

            // Foreground
            float hpPercent = (float)health / getMaxHealth();
            if (hpPercent < 0) hpPercent = 0;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * hpPercent, barHeight));
            frontBar.setOrigin(barWidth / 2, barHeight / 2);
            frontBar.setPosition(pos.x - (barWidth * (1 - hpPercent)) / 2.0f, pos.y);

            if (hpPercent > 0.5f) frontBar.setFillColor(sf::Color::Green);
            else if (hpPercent > 0.25f) frontBar.setFillColor(sf::Color::Yellow);
            else frontBar.setFillColor(sf::Color::Red);

            window.draw(backBar);
            window.draw(frontBar);
        }
    }

    virtual void renderEffects(sf::RenderWindow& window) {}

    // --- State Management ---
    bool getIsAlive() const { return isAlive; }

    void setSelected(bool status) { isSelected = status; }

    void takeDamage(float amount) {
        health -= amount;
        if (health <= 0) isAlive = false;
    }

    sf::FloatRect getBounds() const {
        return hasTexture ? sprite.getGlobalBounds() : shape.getGlobalBounds();
    }

    virtual std::string stats() = 0;
};

#endif