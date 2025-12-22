#include "Map/MapManager.h"
#include "Game/ResourceManager.h"
#include "Entity System/Entity Type/types.h"
#include "Entity System/Entity Type/House.h"
#include "Entity System/Entity Type/StoneMine.h"
#include "Entity System/Entity Type/Farm.h"
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/Tree.h"
#include "UI/AssetManager.h"

#include <iostream>
#include <ctime>

MapManager::MapManager(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize) {
    m_level.resize(m_width * m_height, 0);
}

void MapManager::initialize() {
    createTilesetFile();

    if (!m_tilesetTexture.loadFromFile("tileset.png")) {
        std::cerr << "HATA: tileset.png yuklenemedi!" << std::endl;
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 1. DUVAR OLUÞTURMA (%3 Oranýnda)
    int totalTiles = m_width * m_height;
    int wallCount = (totalTiles * 3) / 100;

    for (int i = 0; i < wallCount; i++) {
        int rx = std::rand() % m_width;
        int ry = std::rand() % m_height;

        if (m_level[rx + ry * m_width] == 0) {
            m_level[rx + ry * m_width] = 1;
        }
    }

    if (!m_map.load("tileset.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "MapManager load hatasi!" << std::endl;
    }

    // --- DEÐÝÞEN KISIM: 3x3 ORMAN KÜMELERÝ ---

    // Harita büyüklüðüne göre kaç tane orman kümesi olacaðýný belirle
    // Örn: %2 yoðunluk -> 50x50 haritada 50 tane küme oluþturur.
    int forestClusterCount = (totalTiles * 2) / 100;

    for (int i = 0; i < forestClusterCount; i++) {
        // Rastgele bir baþlangýç noktasý seç (Kümeyi baþlatacak sol üst köþe)
        int startX = std::rand() % m_width;
        int startY = std::rand() % m_height;

        // 3x3'lük alaný tara
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                int tx = startX + x;
                int ty = startY + y;

                // Harita sýnýrlarý içinde miyiz?
                if (tx >= 0 && tx < m_width && ty >= 0 && ty < m_height) {

                    int index = tx + ty * m_width;

                    // Eðer zemin boþsa (Duvar veya baþka bina yoksa)
                    if (m_level[index] == 0) {

                        // Küme içindeki her kareye %75 ihtimalle aðaç dik
                        // (Böylece tam kare gibi durmaz, daha doðal ve boþluklu olur)
                        if (std::rand() % 100 < 75) {

                            std::shared_ptr<Tree> newTree = std::make_shared<Tree>();

                            sf::Texture& tex = AssetManager::getTexture("assets/nature/tree.png");
                            newTree->setTexture(tex);

                            // Ölçekleme
                            float targetSize = (float)m_tileSize;
                            sf::Vector2u texSize = tex.getSize();
                            if (texSize.x > 0 && texSize.y > 0) {
                                newTree->setScale(targetSize / texSize.x, targetSize / texSize.y);
                            }

                            // Pozisyon
                            float centerX = (tx * m_tileSize) + (targetSize / 2.0f);
                            float centerY = (ty * m_tileSize) + (targetSize / 2.0f);
                            newTree->setPosition(sf::Vector2f(centerX, centerY));

                            m_buildings.push_back(newTree);

                            // Alaný iþaretle
                            m_level[index] = 1;
                        }
                    }
                }
            }
        }
    }
}

// ... Dosyanýn geri kalaný ayný kalacak ...
void MapManager::createTilesetFile() {
    // ... Eski kodlar ...
    sf::Image tileset;
    int ts = m_tileSize;
    tileset.create(ts * 22, ts, sf::Color::Black);

    // Çimen
    for (unsigned int x = 0; x < ts; ++x) {
        for (unsigned int y = 0; y < ts; ++y) {
            if (x == 0 || y == 0 || x == ts - 1 || y == ts - 1)
                tileset.setPixel(x, y, sf::Color(34, 139, 34));
            else
                tileset.setPixel(x, y, sf::Color(50, 205, 50));
        }
    }
    // Duvar
    for (unsigned int x = ts; x < ts * 2; ++x) {
        for (unsigned int y = 0; y < ts; ++y) {
            tileset.setPixel(x, y, sf::Color(105, 105, 105));
            if ((x - ts) == y || (x - ts) == (ts - 1 - y))
                tileset.setPixel(x, y, sf::Color::Black);
        }
    }
    tileset.saveToFile("tileset.png");
}

bool MapManager::tryPlaceBuilding(int tx, int ty, BuildTypes type) {
    if (tx < 0 || ty < 0 || tx + 1 >= m_width || ty + 1 >= m_height) return false;

    int indices[4] = { tx + ty * m_width, (tx + 1) + ty * m_width, tx + (ty + 1) * m_width, (tx + 1) + (ty + 1) * m_width };
    if (m_level[indices[0]] != 0) return false;

    std::shared_ptr<Building> newBuilding = nullptr;
    std::string textureName = "";
    float widthInTiles = 2.0f;
    float heightInTiles = 2.0f;

    if (type == BuildTypes::House) {
        newBuilding = std::make_shared<House>();
        textureName = "assets/buildings/house.png";
        widthInTiles = 1.0f; heightInTiles = 1.0f;
    }
    else if (type == BuildTypes::Barrack) {
        newBuilding = std::make_shared<Barracks>();
        textureName = "assets/buildings/barracks.png";
        widthInTiles = 2.0f; heightInTiles = 2.0f;
    }

    if (newBuilding) {
        float targetW = widthInTiles * m_tileSize;
        float targetH = heightInTiles * m_tileSize;

        if (!textureName.empty()) {
            sf::Texture& tex = AssetManager::getTexture(textureName);
            newBuilding->setTexture(tex);
            sf::Vector2u texSize = tex.getSize();
            newBuilding->setScale(targetW / texSize.x, targetH / texSize.y);
        }

        float centerX = (tx * m_tileSize) + (targetW / 2.0f);
        float centerY = (ty * m_tileSize) + (targetH / 2.0f);
        newBuilding->setPosition(sf::Vector2f(centerX, centerY));

        m_buildings.push_back(newBuilding);

        for (int x = 0; x < widthInTiles; x++) {
            for (int y = 0; y < heightInTiles; y++) {
                int idx = (tx + x) + (ty + y) * m_width;
                if (idx < m_level.size()) m_level[idx] = 1;
            }
        }
        return true;
    }
    return false;
}

void MapManager::removeBuilding(int tx, int ty) {
    sf::Vector2f checkPos(tx * m_tileSize + 5, ty * m_tileSize + 5);
    for (auto it = m_buildings.begin(); it != m_buildings.end(); ) {
        if ((*it)->getBounds().contains(checkPos)) {
            sf::Vector2f pos = (*it)->getPosition();
            int bx = static_cast<int>(pos.x / m_tileSize);
            int by = static_cast<int>(pos.y / m_tileSize);

            int w = 2, h = 2;
            if ((*it)->buildingType == BuildTypes::House) { w = 1; h = 1; }

            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    updateTile(bx + x, by + y, 0);
                }
            }
            it = m_buildings.erase(it);
            return;
        }
        else { ++it; }
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

const std::vector<int>& MapManager::getLevelData() const { return m_level; }

Building* MapManager::getBuildingAt(int tx, int ty) {
    sf::Vector2f checkPos(tx * m_tileSize + 16, ty * m_tileSize + 16);
    for (auto& b : m_buildings) {
        if (b->getBounds().contains(checkPos)) return b.get();
    }
    return nullptr;
}

void MapManager::updateBuildings(float dt) {}

bool MapManager::isBuildingAt(int tx, int ty) {
    return (getBuildingAt(tx, ty) != nullptr);
}

void MapManager::removeDeadBuildings() {
    auto it = m_buildings.begin();
    while (it != m_buildings.end()) {

        // Eðer bina/aðaç ölü ise
        if (!(*it)->getIsAlive()) {

            // Konumunu bul
            sf::Vector2f pos = (*it)->getPosition();
            int tx = static_cast<int>(pos.x / m_tileSize);
            int ty = static_cast<int>(pos.y / m_tileSize);

            // Aðacýn kapladýðý alaný (1x1) temizle (0 yap)
            int size = 1;
            // Eðer büyük bina ise size'ý deðiþtirebilirsin
            if ((*it)->buildingType == BuildTypes::Barrack) size = 2;

            for (int x = 0; x < size; x++) {
                for (int y = 0; y < size; y++) {
                    // updateTile ile haritadaki engeli kaldýrýyoruz
                    updateTile(tx + x, ty + y, 0);
                }
            }
            // Listeden sil
            it = m_buildings.erase(it);
        }
        else {
            ++it;
        }
    }
}