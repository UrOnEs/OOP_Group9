#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "TileMap.h"

// TÝP TANIMLAMALARINI UNUTMAMAK ÝÇÝN:
#include "Entity System/Entity Type/types.h" 

// Forward Declaration (Döngüsel baðýmlýlýðý önler)
class ResourceManager;
class Building;

class MapManager {
public:
    MapManager(int width, int height, int tileSize);

    void initialize();

    // INPUT'u buradan sildim (Game.cpp yönetecek demiþtik)
    // Sadece komut alacak:

    // --- DÜZELTME: BuildingType -> BuildTypes ---
    bool tryPlaceBuilding(int tx, int ty, BuildTypes type, ResourceManager& resMgr);

    void removeBuilding(int tx, int ty);

    void updateBuildings(float dt); // sf::Time yerine float dt daha pratik
    void draw(sf::RenderWindow& window);

    // Getterlar
    const std::vector<int>& getLevelData() const;
    Building* getBuildingAt(int tx, int ty); // Raw pointer döndürür
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

    // Building listesi
    std::vector<std::shared_ptr<Building>> m_buildings;
    // unique_ptr yerine shared_ptr yaptým, Entity sisteminle daha uyumlu olsun.
};

/*
#pragma once
#include <SFML/Graphics.hpp>
#include "Entity System\Entity Type\Building.h"
#include <vector>
#include <memory> // unique_ptr icin
#include "TileMap.h"
#include "Entity System/Entity Type/types.h"

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
    void addBuilding(int tx, int ty, BuildTypes type, ResourceManager& resMgr);
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

*/