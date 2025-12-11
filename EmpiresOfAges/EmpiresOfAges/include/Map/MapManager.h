#pragma once
#include <SFML/Graphics.hpp>
#include "Entity System\Entity Type\Building.h"
#include <vector>
#include <memory> // unique_ptr icin
#include "TileMap.h"

// ResourceManager'i tanitiyoruz (Forward Declaration)
class ResourceManager;

class MapManager {
public:
    MapManager(int width, int height, int tileSize);

    void initialize();

    // Input isleyicisi
    void handleInput(sf::RenderWindow& window, const sf::View& camera, ResourceManager& resMgr);

    void updateBuildings(sf::Time dt, ResourceManager& resMgr);

    // Cizim fonksiyonu
    void draw(sf::RenderWindow& window);

    // --- GETTER FONKSIYONLARI (Hata veren kisim buranin eksik olmasindan kaynaklaniyor) ---
    const std::vector<int>& getLevelData() const;
    Building* getBuildingAt(int tx, int ty);
    int getWidth() const;
    int getHeight() const;
    sf::Vector2u getTileSize() const; // sf::Vector2u donduruyor

private:
    void createTilesetFile();
    void updateTile(int tx, int ty, int id);

    // Bina ekleme/silme
    void addBuilding(int tx, int ty, BuildingType type, ResourceManager& resMgr);
    void removeBuilding(int tx, int ty);
    bool isBuildingAt(int tx, int ty);

    int m_width;
    int m_height;
    int m_tileSize;

    std::vector<int> m_level;
    TileMap m_map;
    sf::Texture m_tilesetTexture;

    // Bina listesi (Pointer)
    std::vector<std::unique_ptr<Building>> m_buildings;
};