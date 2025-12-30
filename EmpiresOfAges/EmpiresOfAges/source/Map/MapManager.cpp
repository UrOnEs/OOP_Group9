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

#include "UI/AssetManager.h"

#include <iostream>
#include <ctime>

MapManager::MapManager(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize) {
    m_level.resize(m_width * m_height, 0);
}

void MapManager::initialize(unsigned int seed) {
    // 1. ADIM: createTilesetFile() ÇAÐRISINI KALDIRIYORUZ
    // createTilesetFile(); 

    // Tileset texture'ýný yüklemeye gerek kalmadý çünkü TileMap kendi içinde yüklüyor, 
    // ama AssetManager üzerinden önbelleðe almak isterseniz burasý kalabilir.
    // if (!m_tilesetTexture.loadFromFile("tileset.png")) ... (Bunu da silebilir veya deðiþtirebilirsiniz)

    std::srand(seed);

    m_buildings.clear();
    std::fill(m_level.begin(), m_level.end(), 0);

    // 2. ADIM: ARTIK "tileset.png" YERÝNE KENDÝ ASSETÝNÝZÝ YÜKLÜYORUZ
    // Varsayým: "assets/nature/grass.png" adýnda bir çimen görseliniz var.
    // Eðer görseliniz yoksa projeye eklemelisiniz.
    if (!m_map.load("assets/nature/grass.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "HATA: Harita zemini (grass.png) yuklenemedi!" << std::endl;

        // Failsafe: Eðer grass.png yoksa yine eski yönteme dönmek isterseniz:
        // createTilesetFile();
        // m_map.load("tileset.png", ...);
    }

    int totalTiles = m_width * m_height;

    // ... (Geri kalan kodlar: AÐAÇ OLUÞTURMA, TAÞ OLUÞTURMA vb. AYNI KALACAK) ...
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

                        // Dikkat: Burada ID'yi 1 yapýyoruz (Engel).
                        // Eðer grass.png tek karelik bir görselse, TileMap.cpp'de yapacaðýmýz
                        // deðiþiklikle ID 1 olsa bile yine çimen (ID 0) görselini kullanmasýný saðlayacaðýz.
                        m_level[tx + ty * m_width] = 1;
                        m_level[(tx + 1) + ty * m_width] = 1;
                        m_level[tx + (ty + 1) * m_width] = 1;
                        m_level[(tx + 1) + (ty + 1) * m_width] = 1;
                    }
                }
            }
        }
    }

    // ... (TAÞ ve ALTIN oluþturma kodlarý buranýn devamýnda aynen kalacak) ...

    // =========================================================
    // 2. TAÞ OLUÞTURMA (Tek Tek - %0.2 Oranýnda)
    // =========================================================
    int stoneCount = (totalTiles * 2) / 1000;

    for (int i = 0; i < stoneCount; i++) {
        int tx = std::rand() % m_width;
        int ty = std::rand() % m_height;

        if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {
            bool isSpaceFree = true;
            if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

            if (isSpaceFree) {
                std::shared_ptr<Stone> newStone = std::make_shared<Stone>();
                sf::Texture& tex = AssetManager::getTexture("assets/nature/stone.png");
                newStone->setTexture(tex);
                float targetSize = (float)m_tileSize * 2.0f;
                sf::Vector2u texSize = tex.getSize();
                if (texSize.x > 0 && texSize.y > 0) {
                    newStone->setScale(targetSize / texSize.x, targetSize / texSize.y);
                }
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
    // 3. ALTIN OLUÞTURMA
    // =========================================================
    int goldCount = (totalTiles * 1) / 1000;
    for (int i = 0; i < goldCount; i++) {
        int tx = std::rand() % m_width;
        int ty = std::rand() % m_height;
        if (tx >= 0 && tx + 1 < m_width && ty >= 0 && ty + 1 < m_height) {
            bool isSpaceFree = true;
            if (m_level[tx + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + ty * m_width] != 0) isSpaceFree = false;
            else if (m_level[tx + (ty + 1) * m_width] != 0) isSpaceFree = false;
            else if (m_level[(tx + 1) + (ty + 1) * m_width] != 0) isSpaceFree = false;

            if (isSpaceFree) {
                std::shared_ptr<Gold> newGold = std::make_shared<Gold>();
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

// createTilesetFile fonksiyonunu tamamen silebilirsiniz veya boþ býrakabilirsiniz.
void MapManager::createTilesetFile() {
    // KULLANILMIYOR
}

// ... (tryPlaceBuilding, removeBuilding vb. fonksiyonlar DEÐÝÞMEDEN KALACAK) ...
std::shared_ptr<Building> MapManager::tryPlaceBuilding(int tx, int ty, BuildTypes type) {
    // (Orijinal koddaki içerik aynen kalacak)
    if (tx < 0 || ty < 0 || tx + 1 >= m_width || ty + 1 >= m_height) return nullptr;
    float widthInTiles = 4.0f;
    float heightInTiles = 4.0f;
    std::shared_ptr<Building> newBuilding = nullptr;
    std::string textureName = "";
    if (type == BuildTypes::House) {
        newBuilding = std::make_shared<House>();
        textureName = "assets/buildings/house.png";
        widthInTiles = 2.0f;
        heightInTiles = 2.0f;
    }
    else if (type == BuildTypes::Barrack) {
        newBuilding = std::make_shared<Barracks>();
        textureName = "assets/buildings/barrack.png";
        widthInTiles = 4.0f;
        heightInTiles = 4.0f;
    }
    else if (type == BuildTypes::Farm) {
        newBuilding = std::make_shared<Farm>();
        textureName = "assets/buildings/mill.png";
        widthInTiles = 4.0f;
        heightInTiles = 4.0f;
    }
    else if (type == BuildTypes::TownCenter) {
        newBuilding = std::make_shared<TownCenter>();
        textureName = "assets/buildings/castle.png";
        widthInTiles = 6.0f;
        heightInTiles = 6.0f;
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
    // (Orijinal koddaki içerik aynen kalacak)
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
    // tileset.png yazýyor olsa bile TileMap.cpp dosyasýnda "load" çaðrýsýnda
    // hangi texture yüklendiyse onu kullanýr. Buradaki string parametresi
    // aslýnda TileMap::updateTile fonksiyonunda sadece texture yenileme durumunda kullanýlýyor.
    // Þimdilik "assets/nature/grass.png" diyebiliriz veya boþ geçebiliriz,
    // ancak TileMap::updateTile içindeki mantýk önemlidir.
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
            // SÝLÝNEN BÝNANIN ALTINDAKÝ ALANI TAMAMEN TEMÝZLE
            // Sadece clearRect ile kesiþen kýsmý deðil, binanýn kendi kapladýðý alaný sýfýrla.

            sf::FloatRect bBounds = (*it)->getBounds();
            int bx = static_cast<int>(bBounds.left / m_tileSize);
            int by = static_cast<int>(bBounds.top / m_tileSize);

            int bw = 4, bh = 4; // Varsayýlan boyut

            // Boyut belirleme (removeDeadBuildings mantýðýyla ayný)
            if ((*it)->buildingType == BuildTypes::House) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::TownCenter) { bw = 6; bh = 6; }
            else if ((*it)->buildingType == BuildTypes::Tree) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::Stone) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::Gold) { bw = 2; bh = 2; }
            else if ((*it)->buildingType == BuildTypes::Farm) { bw = 4; bh = 4; }
            else if ((*it)->buildingType == BuildTypes::Barrack) { bw = 4; bh = 4; }

            // Binanýn kapladýðý bütün kareleri boþalt (0 yap)
            for (int x = 0; x < bw; x++) {
                for (int y = 0; y < bh; y++) {
                    updateTile(bx + x, by + y, 0);
                }
            }

            // Þimdi binayý listeden sil
            it = m_buildings.erase(it);
        }
        else {
            ++it;
        }
    }

    // Seçilen dikdörtgen alaný garanti temizle (Zemin texture'ýný düzeltmek için)
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            updateTile(startX + x, startY + y, 0);
        }
    }
}