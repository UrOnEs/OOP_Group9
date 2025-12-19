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

#include "UI/AssetManager.h"

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

bool MapManager::tryPlaceBuilding(int tx, int ty, BuildTypes type) {
    // 1. Sýnýr ve Doluluk Kontrolü (Aynen kalsýn)
    if (tx < 0 || ty < 0 || tx + 1 >= m_width || ty + 1 >= m_height) return false;

    // Basitlik için 1x1 alan kontrolü yapýyoruz (Ev için).
    // Eðer büyük bina koyacaksan buradaki döngüyü binanýn boyutuna göre güncellemelisin.
    int indices[4] = { tx + ty * m_width, (tx + 1) + ty * m_width, tx + (ty + 1) * m_width, (tx + 1) + (ty + 1) * m_width };
    // Þimdilik sadece sol üst köþe dolu mu diye bakalým (House 1x1 olduðu için)
    if (m_level[indices[0]] != 0) return false;

    std::shared_ptr<Building> newBuilding = nullptr;
    std::string textureName = "";

    // --- BOYUT AYARLARI ---
    float widthInTiles = 2.0f; // Varsayýlan (Kýþla vb.)
    float heightInTiles = 2.0f;

    if (type == BuildTypes::House) {
        newBuilding = std::make_shared<House>();
        textureName = "assets/buildings/house.png"; // Senin dosya adýn
        widthInTiles = 1.0f; // Ev 1x1
        heightInTiles = 1.0f;
    }
    else if (type == BuildTypes::Barrack) {
        newBuilding = std::make_shared<Barracks>();
        textureName = "assets/buildings/barracks.png";
        widthInTiles = 2.0f; // Kýþla 2x2
        heightInTiles = 2.0f;
    }
    // ... Diðer binalar ...

    if (newBuilding) {
        // --- 1. ÖLÇEKLEME (SCALE) ---
        // Hedef boyut (Piksel cinsinden)
        float targetW = widthInTiles * m_tileSize;
        float targetH = heightInTiles * m_tileSize;

        if (!textureName.empty()) {
            sf::Texture& tex = AssetManager::getTexture(textureName);
            newBuilding->setTexture(tex);

            // Resmi hedef boyuta sýðdýr
            sf::Vector2u texSize = tex.getSize();
            newBuilding->setScale(targetW / texSize.x, targetH / texSize.y);
        }

        // --- 2. POZÝSYONLAMA (CENTERING) ---
        // Entity::setTexture fonksiyonu Origin'i resmin MERKEZÝNE (Center) alýyor.
        // Bu yüzden pozisyonu da karenin MERKEZÝNE vermeliyiz.

        float centerX = (tx * m_tileSize) + (targetW / 2.0f);
        float centerY = (ty * m_tileSize) + (targetH / 2.0f);

        newBuilding->setPosition(sf::Vector2f(centerX, centerY));

        m_buildings.push_back(newBuilding);

        // Haritada alaný iþaretle
        // (Eðer bina 1x1 ise sadece 1 kareyi, 2x2 ise 4 kareyi iþaretle)
        for (int x = 0; x < widthInTiles; x++) {
            for (int y = 0; y < heightInTiles; y++) {
                int idx = (tx + x) + (ty + y) * m_width;
                if (idx < m_level.size()) {
                    m_level[idx] = 1;
                    // updateTile(tx+x, ty+y, 1); // Ýstersen gri duvar görselini açabilirsin
                }
            }
        }

        return true;
    }
    return false;
}

// --------------- SÝLME MANTIÐI ---------------------------
void MapManager::removeBuilding(int tx, int ty) {
    // Týklanan noktayý piksel olarak bul (tam orta nokta olmasýn, sol üst köþeye yakýn olsun)
    sf::Vector2f checkPos(tx * m_tileSize + 5, ty * m_tileSize + 5);

    for (auto it = m_buildings.begin(); it != m_buildings.end(); ) {
        // Eðer bina bu noktayý kapsýyorsa
        if ((*it)->getBounds().contains(checkPos)) {

            // Binanýn sol üst grid koordinatýný bul
            sf::Vector2f pos = (*it)->getPosition();
            int bx = static_cast<int>(pos.x / m_tileSize);
            int by = static_cast<int>(pos.y / m_tileSize);

            // Binanýn boyutuna göre altýndaki kareleri temizle (0 yap)
            int widthInTiles = 2; // Varsayýlan
            int heightInTiles = 2;

            if ((*it)->buildingType == BuildTypes::House) {
                widthInTiles = 1;
                heightInTiles = 1;
            }

            for (int x = 0; x < widthInTiles; x++) {
                for (int y = 0; y < heightInTiles; y++) {
                    updateTile(bx + x, by + y, 0); // 0 = Çimen/Boþ
                }
            }

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