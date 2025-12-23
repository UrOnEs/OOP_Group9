#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "TileMap.h"
#include "Entity System/Entity Type/types.h"
#include "UI/AssetManager.h"

class ResourceManager;
class Building;

class MapManager {
public:
    MapManager(int width, int height, int tileSize);

    void initialize();
    std::shared_ptr<Building> tryPlaceBuilding(int tx, int ty, BuildTypes type);
    void removeBuilding(int tx, int ty);
    void updateBuildings(float dt);
    void draw(sf::RenderWindow& window);
    void removeDeadBuildings();


    const std::vector<int>& getLevelData() const;
    std::shared_ptr<Building> getBuildingAt(int tx, int ty);
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileSize() const { return m_tileSize; }

    // --- EKLENEN FONKSÝYON ---
    // Game.cpp'nin binalarý/aðaçlarý güncelleyebilmesi için listeyi veriyoruz
    std::vector<std::shared_ptr<Building>>& getBuildings() { return m_buildings; }
    // -------------------------

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