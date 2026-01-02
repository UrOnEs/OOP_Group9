#include "Map/TileMap.h"

bool TileMap::load(const std::string& tilesetFile, sf::Vector2u tileSize, const std::vector<int>& mapData, unsigned int width, unsigned int height) {
    if (!m_tileset.loadFromFile(tilesetFile))
        return false;

    // Smooth false for pixel art style, true for smoother scaling
    m_tileset.setSmooth(true);
    m_tileset.setRepeated(true);

    m_width = width;
    m_tileSize = tileSize;
    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize(width * height * 4);

    // Calculate tileset dimensions in tiles
    int tilesetWidthInTiles = m_tileset.getSize().x / tileSize.x;
    int tilesetHeightInTiles = m_tileset.getSize().y / tileSize.y;

    // Safety check for single-tile textures
    if (tilesetWidthInTiles == 0) tilesetWidthInTiles = 1;
    if (tilesetHeightInTiles == 0) tilesetHeightInTiles = 1;

    for (unsigned int i = 0; i < width; ++i) {
        for (unsigned int j = 0; j < height; ++j) {
            int tileNumber = mapData[i + j * width];

            sf::Vertex* quad = &m_vertices[(i + j * width) * 4];

            quad[0].position = sf::Vector2f(i * tileSize.x, j * tileSize.y);
            quad[1].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
            quad[2].position = sf::Vector2f((i + 1) * tileSize.x, (j + 1) * tileSize.y);
            quad[3].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);

            // Safe texture coordinate calculation
            int tu = tileNumber % tilesetWidthInTiles;
            int tv = tileNumber / tilesetWidthInTiles;

            // Clamp check to prevent out-of-bounds access
            if (tv >= tilesetHeightInTiles) {
                tv = 0;
                tu = 0;
            }

            quad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
            quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
            quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
            quad[3].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
        }
    }
    return true;
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = &m_tileset;
    target.draw(m_vertices, states);
}

void TileMap::updateTile(int x, int y, int newID, const std::string& tilesetFile) {
    if (x < 0 || y < 0 || x >= (m_vertices.getVertexCount() / 4) / m_tileSize.y) return;

    sf::Vertex* quad = &m_vertices[(x + y * m_width) * 4];

    int tilesetWidthInTiles = m_tileset.getSize().x / m_tileSize.x;
    int tilesetHeightInTiles = m_tileset.getSize().y / m_tileSize.y;

    if (tilesetWidthInTiles == 0) tilesetWidthInTiles = 1;
    if (tilesetHeightInTiles == 0) tilesetHeightInTiles = 1;

    int tu = newID % tilesetWidthInTiles;
    int tv = newID / tilesetWidthInTiles;

    if (tv >= tilesetHeightInTiles) {
        tv = 0;
        tu = 0;
    }

    quad[0].texCoords = sf::Vector2f(tu * m_tileSize.x, tv * m_tileSize.y);
    quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, tv * m_tileSize.y);
    quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, (tv + 1) * m_tileSize.y);
    quad[3].texCoords = sf::Vector2f(tu * m_tileSize.x, (tv + 1) * m_tileSize.y);
}