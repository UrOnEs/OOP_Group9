// TileMap.h

#ifndef TILEMAP_H
#define TILEMAP_H

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class TileMap : public sf::Drawable, public sf::Transformable {
public:
    bool load(const std::string& tilesetFile, sf::Vector2u tileSize, const std::vector<int>& mapData, unsigned int width, unsigned int height);
    void updateTile(int x, int y, int newID, const std::string& tilesetFile);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::VertexArray m_vertices;
    sf::Texture m_tileset;
    sf::Vector2u m_tileSize;
    unsigned int m_width;
};
#endif