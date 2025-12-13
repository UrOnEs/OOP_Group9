#include "Map/MapManager.h"
#include "Game/ResourceManager.h"
#include "Entity System/Entity Type/types.h"

// Alt Sýnýflar
#include "Entity System/Entity Type/House.h"
#include "Entity System/Entity Type/StoneMine.h"
#include "Entity System/Entity Type/Farm.h"
#include "Entity System/Entity Type/Barracks.h"
// #include "Entity System/Entity Type/LumberCamp.h" // Dosya yoksa kapalý kalsýn
// #include "Entity System/Entity Type/GoldMine.h"   // Dosya yoksa kapalý kalsýn

#include <iostream>
#include <ctime>

MapManager::MapManager(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize) {
    m_level.resize(m_width * m_height, 0);
}

void MapManager::initialize() {
    createTilesetFile(); // PNG oluþtur

    if (!m_tilesetTexture.loadFromFile("tileset.png")) {
        std::cerr << "HATA: tileset.png yuklenemedi!" << std::endl;
    }

    // Rastgele Duvarlar (%10 oranýnda)
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < (m_width * m_height) / 10; i++) {
        int rx = std::rand() % m_width;
        int ry = std::rand() % m_height;
        m_level[rx + ry * m_width] = 1;
    }

    // TileMap yükle
    if (!m_map.load("tileset.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "MapManager load hatasi!" << std::endl;
    }
}

void MapManager::createTilesetFile() {
    sf::Image tileset;
    tileset.create(704, 32, sf::Color::Black);

    // 0: Çimen (Yeþil)
    for (unsigned int x = 0; x < 32; ++x) {
        for (unsigned int y = 0; y < 32; ++y) {
            if (x == 0 || y == 0 || x == 31 || y == 31) tileset.setPixel(x, y, sf::Color(34, 139, 34));
            else tileset.setPixel(x, y, sf::Color(50, 205, 50));
        }
    }
    // 1: Duvar (Gri)
    for (unsigned int x = 32; x < 64; ++x) {
        for (unsigned int y = 0; y < 32; ++y) {
            tileset.setPixel(x, y, sf::Color(105, 105, 105));
            if ((x - 32) == y || (x - 32) == (31 - y)) tileset.setPixel(x, y, sf::Color::Black);
        }
    }
    tileset.saveToFile("tileset.png");
}

bool MapManager::tryPlaceBuilding(int tx, int ty, BuildTypes type, ResourceManager& resMgr) {
    if (tx < 0 || ty < 0 || tx + 1 >= m_width || ty + 1 >= m_height) return false;

    int indices[4] = { tx + ty * m_width, (tx + 1) + ty * m_width, tx + (ty + 1) * m_width, (tx + 1) + (ty + 1) * m_width };
    for (int idx : indices) if (m_level[idx] != 0) return false;

    std::shared_ptr<Building> newBuilding = nullptr;

    if (type == BuildTypes::House) newBuilding = std::make_shared<House>();
    else if (type == BuildTypes::Farm) newBuilding = std::make_shared<Farm>();
    else if (type == BuildTypes::StoneMine) newBuilding = std::make_shared<StoneMine>();
    else if (type == BuildTypes::Barrack) newBuilding = std::make_shared<Barracks>();

    if (newBuilding) {
        newBuilding->setPosition(sf::Vector2f(tx * m_tileSize, ty * m_tileSize));
        newBuilding->setTexture(m_tilesetTexture);
        m_buildings.push_back(newBuilding);

        for (int idx : indices) {
            m_level[idx] = 1;
            updateTile(tx, ty, 1);
            updateTile(tx + 1, ty, 1);
            updateTile(tx, ty + 1, 1);
            updateTile(tx + 1, ty + 1, 1);
        }
        return true;
    }
    return false;
}

void MapManager::removeBuilding(int tx, int ty) {
    sf::Vector2f checkPos(tx * m_tileSize + 16, ty * m_tileSize + 16);
    for (auto it = m_buildings.begin(); it != m_buildings.end(); ) {
        if ((*it)->getBounds().contains(checkPos)) {
            sf::Vector2f pos = (*it)->getPosition();
            int bx = static_cast<int>(pos.x / m_tileSize);
            int by = static_cast<int>(pos.y / m_tileSize);
            updateTile(bx, by, 0); updateTile(bx + 1, by, 0);
            updateTile(bx, by + 1, 0); updateTile(bx + 1, by + 1, 0);
            it = m_buildings.erase(it);
            return;
        }
        else {
            ++it;
        }
    }
}

void MapManager::draw(sf::RenderWindow& window) {
    window.draw(m_map);
    for (auto& b : m_buildings) b->render(window);
}

void MapManager::updateTile(int tx, int ty, int id) {
    if (tx >= 0 && tx < m_width && ty >= 0 && ty < m_height) {
        m_level[tx + ty * m_width] = id;
        m_map.updateTile(tx, ty, id, "tileset.png");
    }
}

// Getterlar ve Yardýmcýlar
const std::vector<int>& MapManager::getLevelData() const { return m_level; }

Building* MapManager::getBuildingAt(int tx, int ty) {
    sf::Vector2f checkPos(tx * m_tileSize + 16, ty * m_tileSize + 16);
    for (auto& b : m_buildings) {
        if (b->getBounds().contains(checkPos)) return b.get();
    }
    return nullptr;
}

void MapManager::updateBuildings(float dt) {
    // ResourceManager'a ihtiyaç varsa parametre olarak alýnmalý
}

bool MapManager::isBuildingAt(int tx, int ty) {
    return (getBuildingAt(tx, ty) != nullptr);
}