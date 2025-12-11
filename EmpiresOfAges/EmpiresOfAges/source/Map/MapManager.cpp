#include "Map/MapManager.h"
#include "Map/ResourceManager.h"

// YENÝ: Alt sýnýflarýn baþlýk dosyalarýný ekliyoruz
#include "House.h"
#include "LumberCamp.h"
#include "StoneMine.h"
#include "GoldMine.h"
#include "Farm.h"

#include <iostream>
#include <ctime>
#include <algorithm>

// --- CONSTRUCTOR ---
MapManager::MapManager(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize) {
    m_level.resize(m_width * m_height, 0);
}

// --- INITIALIZE ---
void MapManager::initialize() {
    // 1. Tileset dosyasýný oluþtur (Yeni renklerle)
    createTilesetFile();

    // 2. Texture'ý yükle (Binalarýn çizimi için gerekli)
    if (!m_tilesetTexture.loadFromFile("tileset.png")) {
        std::cerr << "HATA: tileset.png texture yuklenemedi!" << std::endl;
    }

    // 3. Rastgele Duvarlar (Harita boþ kalmasýn)
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < 5000; i++) {
        int rx = std::rand() % m_width;
        int ry = std::rand() % m_height;
        m_level[rx + ry * m_width] = 1;
    }

    // 4. TileMap Görselini Yükle
    if (!m_map.load("tileset.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "MapManager tileset hatasi!" << std::endl;
    }
}

// --- TILESET OLUÞTURUCU (Tüm Binalar Ýçin) ---
void MapManager::createTilesetFile() {
    sf::Image tileset;
    // Geniþliði artýrýyoruz: 22 kare * 32 = 704 piksel (ID 0'dan 21'e kadar)
    // 0-1 (Zemin/Duvar) + 4 Bina * 4 Parça = 18 ID.
    // +4 Parça Farm için = 22 ID eder.
    tileset.create(704, 32, sf::Color::Black);

    // 0: Çimen
    for (unsigned int x = 0; x < 32; ++x) {
        for (unsigned int y = 0; y < 32; ++y) {
            if (x == 0 || y == 0 || x == 31 || y == 31) tileset.setPixel(x, y, sf::Color(34, 139, 34));
            else tileset.setPixel(x, y, sf::Color(50, 205, 50));
        }
    }
    // 1: Duvar
    for (unsigned int x = 32; x < 64; ++x) {
        for (unsigned int y = 0; y < 32; ++y) {
            tileset.setPixel(x, y, sf::Color(105, 105, 105));
            if ((x - 32) == y || (x - 32) == (31 - y)) tileset.setPixel(x, y, sf::Color(0, 0, 0));
        }
    }

    struct BuildColor { int startID; sf::Color main; sf::Color border; };
    std::vector<BuildColor> colors = {
        {2, sf::Color(139, 69, 19), sf::Color(101, 67, 33)},    // House
        {6, sf::Color(34, 100, 34), sf::Color(0, 60, 0)},       // Lumber
        {10, sf::Color(120, 120, 140), sf::Color(80, 80, 100)}, // Stone
        {14, sf::Color(218, 165, 32), sf::Color(184, 134, 11)}, // Gold
        {18, sf::Color(240, 230, 140), sf::Color(189, 183, 107)} // FARM
    };

    for (const auto& c : colors) {
        for (unsigned int i = 0; i < 4; ++i) { // Her bina 4 parça
            int currentID = c.startID + i;
            unsigned int startX = currentID * 32;

            for (unsigned int x = startX; x < startX + 32; ++x) {
                for (unsigned int y = 0; y < 32; ++y) {
                    tileset.setPixel(x, y, c.main);

                    int localX = x - startX;
                    // Kenarlýk Çizimi (Binanýn bütün görünmesi için)
                    bool border = false;
                    if (i == 0 && (y == 0 || localX == 0)) border = true; // Sol-Üst
                    if (i == 1 && (y == 0 || localX == 31)) border = true;// Sað-Üst
                    if (i == 2 && (y == 31 || localX == 0)) border = true;// Sol-Alt
                    if (i == 3 && (y == 31 || localX == 31)) border = true;// Sað-Alt

                    if (border) tileset.setPixel(x, y, c.border);
                    if ((localX + y) % 8 == 0) tileset.setPixel(x, y, c.border); // Desen
                }
            }
        }
    }
    tileset.saveToFile("tileset.png");
}

// --- INPUT YÖNETÝMÝ ---
void MapManager::handleInput(sf::RenderWindow& window, const sf::View& camera, ResourceManager& resMgr) {
    if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        // Tuþlar
        bool keyQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Q); // Duvar
        bool keyW = sf::Keyboard::isKeyPressed(sf::Keyboard::W); // HOUSE
        bool keyE = sf::Keyboard::isKeyPressed(sf::Keyboard::E); // LUMBER
        bool keyR = sf::Keyboard::isKeyPressed(sf::Keyboard::R); // STONE
        bool keyT = sf::Keyboard::isKeyPressed(sf::Keyboard::T); // GOLD
        bool keyF = sf::Keyboard::isKeyPressed(sf::Keyboard::F); // FARM
        bool keyShift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift); // SÝLME

        if (keyQ || keyW || keyE || keyR || keyT || keyShift || keyF) {
            sf::Vector2f mousePosWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), camera);
            int tx = static_cast<int>(mousePosWorld.x / m_tileSize);
            int ty = static_cast<int>(mousePosWorld.y / m_tileSize);

            if (tx >= 0 && tx < m_width && ty >= 0 && ty < m_height) {
                int index = tx + ty * m_width;

                // --- SÝLME (SHIFT) ---
                if (keyShift) {
                    // Önce bina var mý diye bak (Pointer ile kontrol)
                    if (isBuildingAt(tx, ty)) {
                        removeBuilding(tx, ty);
                    }
                    // Yoksa ve o kare duvarsa duvarý sil
                    else if (m_level[index] == 1) {
                        updateTile(tx, ty, 0);
                    }
                }
                // --- DUVAR (Q) ---
                else if (keyQ) {
                    // Sadece boþsa duvar koy
                    if (m_level[index] == 0 && !isBuildingAt(tx, ty)) {
                        updateTile(tx, ty, 1);
                    }
                }
                // --- BÝNA ÝNÞAATI ---
                else if (keyW) addBuilding(tx, ty, BuildingType::House, resMgr);
                else if (keyE) addBuilding(tx, ty, BuildingType::LumberCamp, resMgr);
                else if (keyR) addBuilding(tx, ty, BuildingType::StoneMine, resMgr);
                else if (keyT) addBuilding(tx, ty, BuildingType::GoldMine, resMgr);
                else if (keyF) addBuilding(tx, ty, BuildingType::Farm, resMgr);
            }
        }
    }
}

// --- BÝNA EKLEME (Polimorfizm ve Kaynak Kontrolü ile) ---
void MapManager::addBuilding(int tx, int ty, BuildingType type, ResourceManager& resMgr) {
    // 1. Sýnýr Kontrolü
    if (tx + 1 >= m_width || ty + 1 >= m_height) return;

    // 2. Alan Boþluk Kontrolü
    int indices[4] = {
        tx + ty * m_width, (tx + 1) + ty * m_width,
        tx + (ty + 1) * m_width, (tx + 1) + (ty + 1) * m_width
    };

    bool areaClear = true;
    for (int idx : indices) if (m_level[idx] != 0) areaClear = false;

    if (areaClear) {
        bool built = false;
        std::unique_ptr<Building> newBuilding;

        // Her binanýn maliyeti ve sýnýfý farklý
        if (type == BuildingType::House) {
            if (resMgr.spendResources(100, 0, 0, 0)) { // 100 Odun
                newBuilding = std::make_unique<House>(tx, ty, m_tilesetTexture);
                built = true;
                std::cout << "House Insa Edildi (-100 Odun)\n";
            }
        }
        else if (type == BuildingType::LumberCamp) {
            if (resMgr.spendResources(50, 0, 0, 0)) { // 50 Odun
                newBuilding = std::make_unique<LumberCamp>(tx, ty, m_tilesetTexture);
                built = true;
                std::cout << "LumberCamp Insa Edildi (-50 Odun)\n";
            }
        }
        else if (type == BuildingType::StoneMine) {
            if (resMgr.spendResources(150, 0, 0, 0)) { // 150 Odun
                newBuilding = std::make_unique<StoneMine>(tx, ty, m_tilesetTexture);
                built = true;
                std::cout << "StoneMine Insa Edildi (-150 Odun)\n";
            }
        }
        else if (type == BuildingType::GoldMine) {
            if (resMgr.spendResources(200, 50, 0, 0)) { // 200 Odun, 50 Taþ
                newBuilding = std::make_unique<GoldMine>(tx, ty, m_tilesetTexture);
                built = true;
                std::cout << "GoldMine Insa Edildi (-200 Odun, -50 Tas)\n";
            }
        }
        else if (type == BuildingType::Farm) { // Maliyet: 100 Odun
            if (resMgr.spendResources(100, 0, 0, 0)) {
                newBuilding = std::make_unique<Farm>(tx, ty, m_tilesetTexture);
                built = true;
                std::cout << "Farm Insa Edildi (-100 Odun)\n";
            }
        }

        // Eðer kaynak yetti ve bina oluþturulduysa listeye ekle
        if (built && newBuilding) {
            m_buildings.push_back(std::move(newBuilding)); // Listeye taþý (move)

            // Harita verisini güncelle (Duvar yap ki içinden geçilmesin)
            for (int idx : indices) m_level[idx] = 1;
        }
        else if (!built) {
            std::cout << "Yetersiz Kaynak!\n";
        }
    }
}

// --- BÝNA SÝLME ---
void MapManager::removeBuilding(int tx, int ty) {
    for (auto it = m_buildings.begin(); it != m_buildings.end(); ) {
        // (*it) -> unique_ptr olduðu için pointer'a eriþim saðlar
        // ->getGridBounds() -> Nesnenin metoduna eriþir
        if ((*it)->getGridBounds().contains(tx, ty)) {

            // Binanýn olduðu alaný temizle (0 yap)
            int bx = (*it)->getX();
            int by = (*it)->getY();

            m_level[bx + by * m_width] = 0;
            m_level[(bx + 1) + by * m_width] = 0;
            m_level[bx + (by + 1) * m_width] = 0;
            m_level[(bx + 1) + (by + 1) * m_width] = 0;

            // Listeden sil
            it = m_buildings.erase(it);
            return; // Tek seferde bir bina sil
        }
        else {
            ++it;
        }
    }
}

// --- TILE GÜNCELLEME (Duvarlar için) ---
void MapManager::updateTile(int tx, int ty, int id) {
    if (tx >= 0 && tx < m_width && ty >= 0 && ty < m_height) {
        m_level[tx + ty * m_width] = id;
        m_map.updateTile(tx, ty, id, "tileset.png");
    }
}

// --- HELPER: BÝNANIN POINTERINI DÖNDÜR ---
Building* MapManager::getBuildingAt(int tx, int ty) {
    for (const auto& b : m_buildings) {
        if (b->getGridBounds().contains(tx, ty)) return b.get(); // Raw pointer döndür
    }
    return nullptr;
}

// --- HELPER: BURADA BÝNA VAR MI? ---
bool MapManager::isBuildingAt(int tx, int ty) {
    for (const auto& b : m_buildings) {
        if (b->getGridBounds().contains(tx, ty)) {
            return true;
        }
    }
    return false;
}

// --- ÇÝZÝM ---
void MapManager::draw(sf::RenderWindow& window) {
    // 1. Önce zemini ve duvarlarý çiz
    window.draw(m_map);

    // 2. Üzerine binalarý çiz
    for (const auto& b : m_buildings) {
        window.draw(*b);
    }
}

// --- GETTERLAR ---
const std::vector<int>& MapManager::getLevelData() const { return m_level; }
int MapManager::getWidth() const { return m_width; }
int MapManager::getHeight() const { return m_height; }
sf::Vector2u MapManager::getTileSize() const {
    return sf::Vector2u(static_cast<unsigned int>(m_tileSize), static_cast<unsigned int>(m_tileSize));
}

void MapManager::updateBuildings(sf::Time dt, ResourceManager& resMgr) {
    for (auto& building : m_buildings) {
        building->update(dt, resMgr);
    }
}