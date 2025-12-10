// TileMap.cpp
#include "TileMap.h"

bool TileMap::load(const std::string& tilesetFile, sf::Vector2u tileSize, const std::vector<int>& mapData, unsigned int width, unsigned int height) {
    if (!m_tileset.loadFromFile(tilesetFile))
        return false;

    m_width = width;
    m_tileSize = tileSize;
    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize(width * height * 4);

    for (unsigned int i = 0; i < width; ++i) {
        for (unsigned int j = 0; j < height; ++j) {
            int tileNumber = mapData[i + j * width];

            sf::Vertex* quad = &m_vertices[(i + j * width) * 4];

            quad[0].position = sf::Vector2f(i * tileSize.x, j * tileSize.y);
            quad[1].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
            quad[2].position = sf::Vector2f((i + 1) * tileSize.x, (j + 1) * tileSize.y);
            quad[3].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);

            int tu = tileNumber % (m_tileset.getSize().x / tileSize.x);
            int tv = tileNumber / (m_tileset.getSize().x / tileSize.x);

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
    if (x < 0 || y < 0 || x >= (m_vertices.getVertexCount() / 4) / m_tileSize.y) return; // Basit sýnýr kontrolü (geliþtirilebilir)

    // Vertex dizisindeki ilgili 4 noktayý bul
    // Not: Harita geniþliðini sýnýf içinde saklamadýðýmýz için width'i dýþarýdan bilmemiz veya
    // sýnýf içinde m_width diye saklamamýz gerekir.
    // Þimdilik daha basit bir yöntem: TileMap sýnýfýna m_width'i load fonksiyonunda kaydettirelim.

    // --- DÜZELTME: TileMap sýnýfýnýn header'ýna "unsigned int m_width;" eklediðini varsayýyorum ---

    sf::Vertex* quad = &m_vertices[(x + y * m_width) * 4];

    // Texture koordinatlarýný yeniden hesapla
    int tu = newID % (m_tileset.getSize().x / m_tileSize.x);
    int tv = newID / (m_tileset.getSize().x / m_tileSize.x);

    quad[0].texCoords = sf::Vector2f(tu * m_tileSize.x, tv * m_tileSize.y);
    quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, tv * m_tileSize.y);
    quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, (tv + 1) * m_tileSize.y);
    quad[3].texCoords = sf::Vector2f(tu * m_tileSize.x, (tv + 1) * m_tileSize.y);
}