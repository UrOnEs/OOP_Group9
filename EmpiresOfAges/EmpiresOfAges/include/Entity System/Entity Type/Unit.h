#pragma once
#include "Entity System/Entity.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Map/Point.h"

/**
 * @brief Base class for mobile entities. Handles pathfinding movement and collision.
 */
class Unit : public Entity {
public:
    Unit();
    Unit(int startGridX, int startGridY, int tileSize);

    /**
     * @brief Updates movement logic based on path and map collisions.
     */
    void update(float dt, const std::vector<int>& mapData, int mapWidth, int mapHeight);

    void render(sf::RenderWindow& window) override;

    void setPath(const std::vector<sf::Vector2f>& pathPoints);
    void moveTo(sf::Vector2f targetWorldPos);
    Point getGridPoint() const;
    static int getPopulation() { return entityCounter; };
    float getSpeed() const { return travelSpeed; }

protected:
    float travelSpeed;
    static int entityCounter;

    bool m_isMoving;
    sf::Vector2f m_targetPos;
    int m_tileSize;

    std::vector<sf::Vector2f> m_path;

private:
    bool checkCollision(const sf::Vector2f& newPos, const std::vector<int>& mapData, int width, int height);
};