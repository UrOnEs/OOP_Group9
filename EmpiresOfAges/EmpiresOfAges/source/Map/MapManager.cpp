#include "Map/MapManager.h"
#include "Game/ResourceManager.h"
#include "Entity System/Entity Type/types.h"
#include "Entity System/Entity Type/House.h"
#include "Entity System/Entity Type/StoneMine.h"
#include "Entity System/Entity Type/Farm.h"
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Entity System/Entity Type/Tree.h"
#include "Entity System/Entity Type/Stone.h" // <-- YENÝ: Include eklendi
#include "Entity System/Entity Type/Gold.h" // <-- YENÝ: Include eklendi

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

    if (!m_map.load("tileset.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "MapManager load hatasi!" << std::endl;
    }

    int totalTiles = m_width * m_height;

    // =========================================================
    // 1. AÐAÇ OLUÞTURMA (Gruplar Halinde - %0.5)
    // =========================================================
    int forestClusterCount = (totalTiles * 5) / 1000;

    for (int i = 0; i < forestClusterCount; i++) {
        int startX = std::rand() % m_width;
        int startY = std::rand() % m_height;

        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                int tx = startX + x * 2;
                int ty = startY + y * 2;

                if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {
                    bool isSpaceFree = true;
                    if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
                    else if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
                    else if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
                    else if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

                    if (isSpaceFree && (std::rand() % 100 < 75)) {
                        std::shared_ptr<Tree> newTree = std::make_shared<Tree>();
                        sf::Texture& tex = AssetManager::getTexture("assets/nature/tree.png");
                        newTree->setTexture(tex);

                        float targetSize = (float)m_tileSize * 2.0f;
                        sf::Vector2u texSize = tex.getSize();
                        if (texSize.x > 0 && texSize.y > 0) {
                            newTree->setScale(targetSize / texSize.x, targetSize / texSize.y);
                        }

                        float centerX = (tx * m_tileSize) + (targetSize / 2.0f);
                        float centerY = (ty * m_tileSize) + (targetSize / 2.0f);
                        newTree->setPosition(sf::Vector2f(centerX, centerY));

                        m_buildings.push_back(newTree);

                        m_level[tx + ty * m_width] = 1;
                        m_level[(tx + 1) + ty * m_width] = 1;
                        m_level[tx + (ty + 1) * m_width] = 1;
                        m_level[(tx + 1) + (ty + 1) * m_width] = 1;
                    }
                }
            }
        }
    }

    // =========================================================
    // 2. TAÞ OLUÞTURMA (Tek Tek - %0.2 Oranýnda)
    // =========================================================
    // Binde 2 oranýnda taþ ekleyelim (Aðaçtan daha az olsun)
    int stoneCount = (totalTiles * 2) / 1000;

    for (int i = 0; i < stoneCount; i++) {
        // Rastgele bir konum seç
        int tx = std::rand() % m_width;
        int ty = std::rand() % m_height;

        // Taþ boyutu da 2x2 olsun (Görünür olmasý için)
        // Harita sýnýrlarýný kontrol et
        if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {

            // 2x2 alan boþ mu kontrol et
            bool isSpaceFree = true;
            if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

            if (isSpaceFree) {
                std::shared_ptr<Stone> newStone = std::make_shared<Stone>();
                // Varsa stone.png, yoksa yine tree.png veya baþka bir placeholder kullanýn
                sf::Texture& tex = AssetManager::getTexture("assets/nature/stone.png");
                newStone->setTexture(tex);

                // --- GÖRSEL VE FÝZÝKSEL BOYUT: 2x2 ---
                float targetSize = (float)m_tileSize * 2.0f;
                sf::Vector2u texSize = tex.getSize();
                if (texSize.x > 0 && texSize.y > 0) {
                    newStone->setScale(targetSize / texSize.x, targetSize / texSize.y);
                }

                // Pozisyon (Merkeze hizala)
                float centerX = (tx * m_tileSize) + (targetSize / 2.0f);
                float centerY = (ty * m_tileSize) + (targetSize / 2.0f);
                newStone->setPosition(sf::Vector2f(centerX, centerY));

                m_buildings.push_back(newStone);

                // Haritada alaný iþaretle (Yürünemez)
                m_level[tx + ty * m_width] = 1;
                m_level[(tx + 1) + ty * m_width] = 1;
                m_level[tx + (ty + 1) * m_width] = 1;
                m_level[(tx + 1) + (ty + 1) * m_width] = 1;
            }
        }
    }

    // =========================================================
    // 3. ALTIN OLUÞTURMA (Tek Tek - %0.1 Oranýnda - Çok Nadir)
    // =========================================================
    // Binde 1 oranýnda altýn ekleyelim
    int goldCount = (totalTiles * 1) / 1000;

    for (int i = 0; i < goldCount; i++) {
        int tx = std::rand() % m_width;
        int ty = std::rand() % m_height;

        // Altýn boyutu 2x2
        if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {
            bool isSpaceFree = true;
            if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

            if (isSpaceFree) {
                std::shared_ptr<Gold> newGold = std::make_shared<Gold>();
                // Varsa gold.png, yoksa stone.png veya placeholder kullanýn
                sf::Texture& tex = AssetManager::getTexture("assets/nature/gold.png");
                newGold->setTexture(tex);

                float targetSize = (float)m_tileSize * 2.0f;
                sf::Vector2u texSize = tex.getSize();
                if (texSize.x > 0 && texSize.y > 0) {
                    newGold->setScale(targetSize / texSize.x, targetSize / texSize.y);
                }

                float centerX = (tx * m_tileSize) + (targetSize / 2.0f);
                float centerY = (ty * m_tileSize) + (targetSize / 2.0f);
                newGold->setPosition(sf::Vector2f(centerX, centerY));

                m_buildings.push_back(newGold);

                m_level[tx + ty * m_width] = 1;
                m_level[(tx + 1) + ty * m_width] = 1;
                m_level[tx + (ty + 1) * m_width] = 1;
                m_level[(tx + 1) + (ty + 1) * m_width] = 1;
            }
        }
    }
}


void MapManager::createTilesetFile() {
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
    // Duvar (Aðaçlarýn altýndaki engel ID'si olarak hala kullanýlýyor)
    for (unsigned int x = ts; x < ts * 2; ++x) {
        for (unsigned int y = 0; y < ts; ++y) {
            tileset.setPixel(x, y, sf::Color(105, 105, 105));
            if ((x - ts) == y || (x - ts) == (ts - 1 - y))
                tileset.setPixel(x, y, sf::Color::Black);
        }
    }
    tileset.saveToFile("tileset.png");
}

std::shared_ptr<Building> MapManager::tryPlaceBuilding(int tx, int ty, BuildTypes type) {
    // 1. Sýnýr Kontrolü
    if (tx < 0 || ty < 0 || tx + 1 >= m_width || ty + 1 >= m_height) return nullptr;

    // --- BOYUTLARI BELÝRLE ---
    float widthInTiles = 4.0f;
    float heightInTiles = 4.0f;

    std::shared_ptr<Building> newBuilding = nullptr;
    std::string textureName = "";

    if (type == BuildTypes::House) {
        newBuilding = std::make_shared<House>();
        textureName = "assets/buildings/house.png";
        // Ev: 2x2
        widthInTiles = 2.0f;
        heightInTiles = 2.0f;
    }
    else if (type == BuildTypes::Barrack) {
        newBuilding = std::make_shared<Barracks>();
        textureName = "assets/buildings/barrack.png";
        // Kýþla: 4x4
        widthInTiles = 4.0f;
        heightInTiles = 4.0f;
    }
    else if (type == BuildTypes::Farm) {
        newBuilding = std::make_shared<Farm>();
        textureName = "assets/buildings/mill.png";
        // Çiftlik: 4x4
        widthInTiles = 4.0f;
        heightInTiles = 4.0f;
    }
    else if (type == BuildTypes::TownCenter) {
        newBuilding = std::make_shared<TownCenter>();
        textureName = "assets/buildings/castle.png";
        // Ana Bina: 6x6
        widthInTiles = 6.0f;
        heightInTiles = 6.0f;
    }

    // 2. Alan Kontrolü
    for (int x = 0; x < widthInTiles; x++) {
        for (int y = 0; y < heightInTiles; y++) {
            if (tx + x >= m_width || ty + y >= m_height) return nullptr;
            int idx = (tx + x) + (ty + y) * m_width;
            if (m_level[idx] != 0) return nullptr;
        }
    }

    if (newBuilding) {
        float targetW = widthInTiles * m_tileSize;
        float targetH = heightInTiles * m_tileSize;

        if (!textureName.empty()) {
            sf::Texture& tex = AssetManager::getTexture(textureName);
            newBuilding->setTexture(tex);

            sf::Vector2u texSize = tex.getSize();
            if (texSize.x > 0 && texSize.y > 0) {
                newBuilding->setScale(targetW / (float)texSize.x, targetH / (float)texSize.y);
            }
        }

        // Pozisyonlama (Merkeze)
        float centerX = (tx * m_tileSize) + (targetW / 2.0f);
        float centerY = (ty * m_tileSize) + (targetH / 2.0f);
        newBuilding->setPosition(sf::Vector2f(centerX, centerY));

        m_buildings.push_back(newBuilding);

        // Haritada alaný iþaretle
        for (int x = 0; x < widthInTiles; x++) {
            for (int y = 0; y < heightInTiles; y++) {
                int mapIdx = (tx + x) + (ty + y) * m_width;
                if (mapIdx < m_level.size()) m_level[mapIdx] = 1;
            }
        }
        return newBuilding;
    }
    return nullptr;
}

void MapManager::removeBuilding(int tx, int ty) {
    sf::Vector2f checkPos(tx * m_tileSize + 5, ty * m_tileSize + 5);
    for (auto it = m_buildings.begin(); it != m_buildings.end(); ) {
        if ((*it)->getBounds().contains(checkPos)) {
            sf::FloatRect bounds = (*it)->getBounds();
            int bx = static_cast<int>(bounds.left / m_tileSize);
            int by = static_cast<int>(bounds.top / m_tileSize);

            int w = 4, h = 4;
            if ((*it)->buildingType == BuildTypes::House) { w = 2; h = 2; }
            else if ((*it)->buildingType == BuildTypes::TownCenter) { w = 6; h = 6; }
            else if ((*it)->buildingType == BuildTypes::Tree) { w = 2; h = 2; }
            else if ((*it)->buildingType == BuildTypes::Stone) { w = 2; h = 2; }
            else if ((*it)->buildingType == BuildTypes::Gold) { w = 2; h = 2; } // <-- YENÝ EKLENDÝ

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

std::shared_ptr<Building> MapManager::getBuildingAt(int tx, int ty) {
    sf::Vector2f checkPos(tx * m_tileSize + 16, ty * m_tileSize + 16);
    for (auto& b : m_buildings) {
        if (b->getBounds().contains(checkPos)) return b;
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

        if (!(*it)->getIsAlive()) {
            sf::FloatRect bounds = (*it)->getBounds();
            int tx = static_cast<int>(bounds.left / m_tileSize);
            int ty = static_cast<int>(bounds.top / m_tileSize);

            int w = 4, h = 4;
            if ((*it)->buildingType == BuildTypes::House) { w = 2; h = 2; }
            else if ((*it)->buildingType == BuildTypes::TownCenter) { w = 6; h = 6; }
            else if ((*it)->buildingType == BuildTypes::Tree) { w = 2; h = 2; }

            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    updateTile(tx + x, ty + y, 0);
                }
            }
            it = m_buildings.erase(it);
        }
        else {
            ++it;
        }
    }
}

void MapManager::clearArea(int startX, int startY, int w, int h) {
    sf::FloatRect clearRect(
        startX * m_tileSize,
        startY * m_tileSize,
        w * m_tileSize,
        h * m_tileSize
    );

    auto it = m_buildings.begin();
    while (it != m_buildings.end()) {
        if ((*it)->getBounds().intersects(clearRect)) {
            it = m_buildings.erase(it);
        }
        else {
            ++it;
        }
    }

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            updateTile(startX + x, startY + y, 0);
        }
    }
}