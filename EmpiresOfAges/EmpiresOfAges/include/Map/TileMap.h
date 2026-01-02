#pragma once
#ifndef TILEMAP_H
#define TILEMAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

/**
 * @brief Handles the rendering of the tile-based map using SFML VertexArrays.
 * Efficiently draws a large grid of tiles using a single draw call.
 */
class TileMap : public sf::Drawable, public sf::Transformable {
public:
    /**
     * @brief Loads the tileset and initializes the map geometry.
     * @param tilesetFile Path to the tileset image.
     * @param tileSize Dimensions of a single tile.
     * @param mapData Vector containing tile IDs for the grid.
     * @param width Width of the map in tiles.
     * @param height Height of the map in tiles.
     * @return true if loading was successful, false otherwise.
     */
    bool load(const std::string& tilesetFile, sf::Vector2u tileSize, const std::vector<int>& mapData, unsigned int width, unsigned int height);

    /**
     * @brief Updates the texture coordinate of a specific tile.
     * @param x X-coordinate of the tile.
     * @param y Y-coordinate of the tile.
     * @param newID The new tile ID (index in the tileset).
     * @param tilesetFile (Optional) Used if tileset needs reloading, currently unused for texture swap.
     */
    void updateTile(int x, int y, int newID, const std::string& tilesetFile);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::VertexArray m_vertices;
    sf::Texture m_tileset;
    sf::Vector2u m_tileSize;
    unsigned int m_width;
};

#endif