#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include "Map/FogOfWar.h"

/**
 * @brief Renders the minimap, including units, terrain, fog of war, and camera view box.
 */
class Minimap {
public:
    Minimap();

    /**
     * @brief Initializes the minimap with map data and screen dimensions.
     */
    void init(int mapWidth, int mapHeight, int tileSize, const std::vector<int>& mapData, int screenWidth, int screenHeight);

    /**
     * @brief Updates unit positions and fog visuals on the minimap.
     */
    void update(const std::vector<std::shared_ptr<Entity>>& myUnits,
        const std::vector<std::shared_ptr<Entity>>& enemyUnits,
        const sf::View& currentView,
        FogOfWar* fog);

    void draw(sf::RenderWindow& window);

    /**
     * @brief Handles clicks on the minimap to move the camera.
     * @param outNewCenter The target world coordinate to move the camera to.
     * @return true if the minimap was clicked.
     */
    bool handleClick(const sf::Vector2i& mousePos, sf::Vector2f& outNewCenter);

    bool isMouseOver(const sf::Vector2i& mousePos) const;

private:
    sf::Vector2f m_size;
    sf::Vector2f m_position;
    float m_scaleX, m_scaleY;

    int m_mapWidth;
    int m_mapHeight;
    int m_tileSize;

    sf::RectangleShape m_background;
    sf::Texture m_mapTexture;
    sf::Sprite m_mapSprite;

    sf::RectangleShape m_cameraBox;
    sf::VertexArray m_unitDots;

    sf::Image m_fogImage;
    sf::Texture m_fogTexture;
    sf::Sprite m_fogSprite;
    sf::Sprite m_frameSprite;
};