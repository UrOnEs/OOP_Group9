#include "Map/MapManager.h"
#include "Game/ResourceManager.h"
#include "Entity System/Entity Type/types.h"
#include "Entity System/Entity Type/House.h"
#include "Entity System/Entity Type/StoneMine.h"
#include "Entity System/Entity Type/Farm.h"
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h"
#include "Entity System/Entity Type/Tree.h"
#include "Entity System/Entity Type/Stone.h"
#include "Entity System/Entity Type/Gold.h"
#include "Entity System/Entity Type/Mountain.h" 

#include "UI/AssetManager.h"

#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>

MapManager::MapManager(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize) {
    m_level.resize(m_width * m_height, 0);
}

void MapManager::initialize(unsigned int seed) {
    std::srand(seed);

    m_buildings.clear();
    std::fill(m_level.begin(), m_level.end(), 0);

    // Load terrain tileset
    if (!m_map.load("assets/nature/grass.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "ERROR: Failed to load map tileset (grass.png)!" << std::endl;
    }

    // =========================================================
    // 0. GENERATE MOUNTAINS & UPDATE VISUALS
    // =========================================================
    createMountains(6);   // Create 6 large mountain clusters
    updateMountainVisuals(); // Autotile logic

    int totalTiles = m_width * m_height;

    // =========================================================
    // 1. GENERATE TREES
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
                        if (texSize.x > 0 && texSize.y > 0) newTree->setScale(targetSize / texSize.x, targetSize / texSize.y);
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
    // 2. GENERATE STONE MINES (2x2 Check)
    // =========================================================
    int stoneCount = (totalTiles * 2) / 1000;
    for (int i = 0; i < stoneCount; i++) {
        int tx = std::rand() % m_width;
        int ty = std::rand() % m_height;

        if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {
            bool isSpaceFree = true;

            if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
            if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
            if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
            if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

            if (isSpaceFree) {
                std::shared_ptr<Stone> newStone = std::make_shared<Stone>();
                sf::Texture& tex = AssetManager::getTexture("assets/nature/stone.png");
                newStone->setTexture(tex);
                float targetSize = (float)m_tileSize * 2.0f;
                sf::Vector2u texSize = tex.getSize();
                if (texSize.x > 0 && texSize.y > 0) newStone->setScale(targetSize / texSize.x, targetSize / texSize.y);
                float centerX = (tx * m_tileSize) + (targetSize / 2.0f);
                float centerY = (ty * m_tileSize) + (targetSize / 2.0f);
                newStone->setPosition(sf::Vector2f(centerX, centerY));
                m_buildings.push_back(newStone);

                m_level[tx + ty * m_width] = 1;
                m_level[(tx + 1) + ty * m_width] = 1;
                m_level[tx + (ty + 1) * m_width] = 1;
                m_level[(tx + 1) + (ty + 1) * m_width] = 1;
            }
        }
    }

    // =========================================================
    // 3. GENERATE GOLD MINES (2x2 Check)
    // =========================================================
    int goldCount = (totalTiles * 1) / 1000;
    for (int i = 0; i < goldCount; i++) {
        int tx = std::rand() % m_width;
        int ty = std::rand() % m_height;

        if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {
            bool isSpaceFree = true;

            if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
            if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
            if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
            if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

            if (isSpaceFree) {
                std::shared_ptr<Gold> newGold = std::make_shared<Gold>();
                sf::Texture& tex = AssetManager::getTexture("assets/nature/gold.png");
                newGold->setTexture(tex);
                float targetSize = (float)m_tileSize * 2.0f;
                sf::Vector2u texSize = tex.getSize();
                if (texSize.x > 0 && texSize.y > 0) newGold->setScale(targetSize / texSize.x, targetSize / texSize.y);
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

// --- ORGANIC MOUNTAIN GENERATION (20x20 Area, %80 Fill) ---
void MapManager::createMountains(int count) {
    int created = 0;
    int attempts = 0;

    while (created < count && attempts < count * 50) {
        attempts++;

        int areaSize = 20;
        int startX = std::rand() % (m_width - areaSize - 4) + 2;
        int startY = std::rand() % (m_height - areaSize - 4) + 2;

        // Base Protection
        float distToPlayer = std::sqrt(std::pow(startX - 6, 2) + std::pow(startY - 5, 2));
        float distToEnemy = std::sqrt(std::pow(startX - 100, 2) + std::pow(startY - 20, 2));

        if (distToPlayer < 40.0f || distToEnemy < 40.0f) continue;

        // Check if area is clear
        bool areaMostlyClear = true;
        for (int x = 0; x < areaSize; x += 2) {
            for (int y = 0; y < areaSize; y += 2) {
                int idx = (startX + x) + (startY + y) * m_width;
                if (m_level[idx] != 0) {
                    areaMostlyClear = false;
                    break;
                }
            }
            if (!areaMostlyClear) break;
        }
        if (!areaMostlyClear) continue;

        // --- ORGANIC GROWTH ALGORITHM ---
        std::vector<bool> localGrid(areaSize * areaSize, false);
        std::vector<sf::Vector2i> frontier;
        std::vector<sf::Vector2i> finalTiles;

        int targetFillCount = (int)(areaSize * areaSize * 0.80f); // 80% fill
        int currentCount = 0;

        int cx = areaSize / 2;
        int cy = areaSize / 2;

        localGrid[cx + cy * areaSize] = true;
        finalTiles.push_back({ cx, cy });
        currentCount++;

        int dx[] = { 0, 0, 1, -1 };
        int dy[] = { 1, -1, 0, 0 };
        for (int i = 0; i < 4; i++) {
            frontier.push_back({ cx + dx[i], cy + dy[i] });
        }

        while (currentCount < targetFillCount && !frontier.empty()) {
            int randIdx = std::rand() % frontier.size();
            sf::Vector2i current = frontier[randIdx];
            frontier.erase(frontier.begin() + randIdx);

            if (current.x < 0 || current.x >= areaSize || current.y < 0 || current.y >= areaSize) continue;
            if (localGrid[current.x + current.y * areaSize]) continue;

            localGrid[current.x + current.y * areaSize] = true;
            finalTiles.push_back(current);
            currentCount++;

            for (int i = 0; i < 4; i++) {
                int nx = current.x + dx[i];
                int ny = current.y + dy[i];
                if (nx >= 0 && nx < areaSize && ny >= 0 && ny < areaSize) {
                    if (!localGrid[nx + ny * areaSize]) {
                        frontier.push_back({ nx, ny });
                    }
                }
            }
        }

        // Add mountains to map
        sf::Texture& tex = AssetManager::getTexture("assets/nature/mountain.png");

        for (auto& tile : finalTiles) {
            int mapX = startX + tile.x;
            int mapY = startY + tile.y;

            if (mapX < 0 || mapX >= m_width || mapY < 0 || mapY >= m_height) continue;
            if (m_level[mapX + mapY * m_width] != 0) continue;

            std::shared_ptr<Mountain> mtn = std::make_shared<Mountain>();
            mtn->setTexture(tex);

            float scale = 1.0f;
            mtn->setScale(scale, scale);

            float posX = (mapX * m_tileSize) + (m_tileSize / 2.0f);
            float posY = (mapY * m_tileSize) + (m_tileSize / 2.0f);
            mtn->setPosition(sf::Vector2f(posX, posY));

            m_buildings.push_back(mtn);
            m_level[mapX + mapY * m_width] = 1; // Mark as obstacle
        }

        created++;
    }
}

// --- VISUAL UPDATE (AUTOTILING) ---
void MapManager::updateMountainVisuals() {
    for (auto& building : m_buildings) {
        if (building->buildingType != BuildTypes::Mountain) continue;

        sf::Vector2f pos = building->getPosition();
        // Convert to grid coordinates
        int tx = static_cast<int>(pos.x / m_tileSize);
        int ty = static_cast<int>(pos.y / m_tileSize);

        // Bitmask Calculation
        int mask = 0;
        // North (1)
        if (ty > 0 && m_level[tx + (ty - 1) * m_width] != 0) mask += 1;
        // West (2)
        if (tx > 0 && m_level[(tx - 1) + ty * m_width] != 0) mask += 2;
        // East (4)
        if (tx < m_width - 1 && m_level[(tx + 1) + ty * m_width] != 0) mask += 4;
        // South (8)
        if (ty < m_height - 1 && m_level[tx + (ty + 1) * m_width] != 0) mask += 8;

        auto mountain = std::dynamic_pointer_cast<Mountain>(building);
        if (mountain) {
            mountain->setVariation(mask, m_tileSize);
        }
    }
}

void MapManager::createTilesetFile() {
    // UNUSED
}

std::shared_ptr<Building> MapManager::tryPlaceBuilding(int tx, int ty, BuildTypes type) {
    if (tx < 0 || ty < 0 || tx + 1 >= m_width || ty + 1 >= m_height) return nullptr;
    float widthInTiles = 4.0f;
    float heightInTiles = 4.0f;
    std::shared_ptr<Building> newBuilding = nullptr;
    std::string textureName = "";
    if (type == BuildTypes::House) {
        newBuilding = std::make_shared<House>();
        textureName = "assets/buildings/house.png";
        widthInTiles = 2.0f; heightInTiles = 2.0f;
    }
    else if (type == BuildTypes::Barrack) {
        newBuilding = std::make_shared<Barracks>();
        textureName = "assets/buildings/barrack.png";
        widthInTiles = 4.0f; heightInTiles = 4.0f;
    }
    else if (type == BuildTypes::Farm) {
        newBuilding = std::make_shared<Farm>();
        textureName = "assets/buildings/mill.png";
        widthInTiles = 4.0f; heightInTiles = 4.0f;
    }
    else if (type == BuildTypes::TownCenter) {
        newBuilding = std::make_shared<TownCenter>();
        textureName = "assets/buildings/castle.png";
        widthInTiles = 6.0f; heightInTiles = 6.0f;
    }
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
        float centerX = (tx * m_tileSize) + (targetW / 2.0f);
        float centerY = (ty * m_tileSize) + (targetH / 2.0f);
        newBuilding->setPosition(sf::Vector2f(centerX, centerY));
        m_buildings.push_back(newBuilding);
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
            else if ((*it)->buildingType == BuildTypes::Gold) { w = 2; h = 2; }
            else if ((*it)->buildingType == BuildTypes::Mountain) { w = 1; h = 1; }

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
        m_map.updateTile(tx, ty, id, "assets/nature/grass.png");
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
            else if ((*it)->buildingType == BuildTypes::Mountain) { w = 1; h = 1; }

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
            sf::FloatRect bBounds = (*it)->getBounds();
            int bx = static_cast<int>(bBounds.left / m_tileSize);
            int by = static_cast<int>(bBounds.top / m_tileSize);

            int bw = 4, bh = 4;
            if ((*it)->buildingType == BuildTypes::House) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::TownCenter) { bw = 6; bh = 6; }
            else if ((*it)->buildingType == BuildTypes::Tree) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::Stone) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::Gold) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::Farm) { bw = 4; bh = 4; }
            else if ((*it)->buildingType == BuildTypes::Barrack) { bw = 4; bh = 4; }
            else if ((*it)->buildingType == BuildTypes::Mountain) { bw = 1; bh = 1; }

            for (int x = 0; x < bw; x++) {
                for (int y = 0; y < bh; y++) {
                    updateTile(bx + x, by + y, 0);
                }
            }
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