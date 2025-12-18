#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "TileMap.h"
#include "Entity System/Entity Type/types.h"

class ResourceManager;
class Building;

class MapManager {
public:
    MapManager(int width, int height, int tileSize);

    void initialize();

    // Bina yerleþtirme denemesi
    bool tryPlaceBuilding(int tx, int ty, BuildTypes type);

    // Bina silme
    void removeBuilding(int tx, int ty);

    // Binalarý çizme ve güncelleme
    void updateBuildings(float dt);
    void draw(sf::RenderWindow& window);

    // Getterlar
    const std::vector<int>& getLevelData() const;
    Building* getBuildingAt(int tx, int ty);
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileSize() const { return m_tileSize; }

private:
    void createTilesetFile();
    void updateTile(int tx, int ty, int id);
    bool isBuildingAt(int tx, int ty);

    int m_width;
    int m_height;
    int m_tileSize;

    std::vector<int> m_level;
    TileMap m_map;
    sf::Texture m_tilesetTexture;
    std::vector<std::shared_ptr<Building>> m_buildings;
};