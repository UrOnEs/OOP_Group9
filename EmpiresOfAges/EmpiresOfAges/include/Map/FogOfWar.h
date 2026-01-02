#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Game/GameRules.h"

/**
 * @brief Represents the visibility state of a tile.
 */
enum class FogState {
    Unexplored, ///< Completely black, never seen before.
    Explored,   ///< Seen previously but not currently visible (dimmed).
    Visible     ///< Currently in line of sight (fully clear).
};

/**
 * @brief Manages the Fog of War system.
 * Handles visibility calculations, rendering of the fog overlay, and exploration state.
 */
class FogOfWar {
public:
    FogOfWar(int width, int height, int tileSize);

    /**
     * @brief Updates fog based on the positions of the player's entities.
     * @param myEntities List of entities owned by the player.
     */
    void update(const std::vector<std::shared_ptr<class Entity>>& myEntities);

    void draw(sf::RenderWindow& window);

    /**
     * @brief Checks if a specific world coordinate is currently visible.
     */
    bool isVisible(float x, float y) const;

    /**
     * @brief Retrieves the fog state at grid coordinates (Used for Minimap).
     */
    FogState getFogAt(int x, int y) const;

private:
    int m_width;
    int m_height;
    int m_tileSize;

    std::vector<FogState> m_grid;
    sf::VertexArray m_vertices;

    void revealArea(int cx, int cy, float radius);
    void updateColor(int x, int y);
};