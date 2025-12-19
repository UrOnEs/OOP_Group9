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
    for (int i = 0; i < (m_width * m_height) / 10; i++) {
        int rx = std::rand() % m_width;
        int ry = std::rand() % m_height;
        m_level[rx + ry * m_width] = 1;
    }

    if (!m_map.load("tileset.png", sf::Vector2u(m_tileSize, m_tileSize), m_level, m_width, m_height)) {
        std::cerr << "MapManager load hatasi!" << std::endl;
    }
}

// --- DEÐÝÞÝKLÝK: DÝNAMÝK TILESET OLUÞTURMA ---
void MapManager::createTilesetFile() {
    sf::Image tileset;

    int ts = m_tileSize; // Kýsaltma

    // Geniþlik hesabý: Yaklaþýk 22 adet tile sýðacak kadar yer ayýrýyoruz.
    // (Eskiden 704px idi, 704/32 = 22 tile).
    tileset.create(ts * 22, ts, sf::Color::Black);

    // 0: Çimen (Yeþil) - [0 ile ts arasý]
    for (unsigned int x = 0; x < ts; ++x) {
        for (unsigned int y = 0; y < ts; ++y) {
            // Kenar çizgileri (Border)
            if (x == 0 || y == 0 || x == ts - 1 || y == ts - 1)
                tileset.setPixel(x, y, sf::Color(34, 139, 34));
            else
                tileset.setPixel(x, y, sf::Color(50, 205, 50));
        }
    }

    // 1: Duvar (Gri) - [ts ile 2*ts arasý]
    for (unsigned int x = ts; x < ts * 2; ++x) {
        for (unsigned int y = 0; y < ts; ++y) {
            tileset.setPixel(x, y, sf::Color(105, 105, 105));

            // X iþareti (Göreceli koordinat hesabý: x - ts)
            if ((x - ts) == y || (x - ts) == (ts - 1 - y))
                tileset.setPixel(x, y, sf::Color::Black);
        }
    }

    // ORMAN OLUÞTURMA (%5 Ýhtimalle Aðaç Koy)
    for (int i = 0; i < (m_width * m_height); i++) {
        // Koordinatlarý bul
        int tx = i % m_width;
        int ty = i / m_width;

        // Eðer zemin boþsa (Duvar veya baþka bina yoksa)
        if (m_level[i] == 0) {
            // %5 ihtimalle aðaç dik
            if (std::rand() % 100 < 5) {
                // tryPlaceBuilding fonksiyonunu "Tree" tipiyle çaðýracaðýz
                // Ancak tryPlaceBuilding'de Tree için özel ayar yapmamýz lazým.
                // O yüzden manuel ekleyelim:

                std::shared_ptr<Tree> newTree = std::make_shared<Tree>();

                // Texture Yükle (Eline bir tree.png resmi lazým)
                sf::Texture& tex = AssetManager::getTexture("assets/nature/tree.png");
                newTree->setTexture(tex);

                // Ölçekle (Aðaçlarý kareye sýðdýr veya biraz taþýrt)
                float targetSize = (float)m_tileSize; // 64px
                sf::Vector2u texSize = tex.getSize();
                newTree->setScale(targetSize / texSize.x, targetSize / texSize.y);

                // Pozisyon (Merkeze)
                float centerX = (tx * m_tileSize) + (targetSize / 2.0f);
                float centerY = (ty * m_tileSize) + (targetSize / 2.0f);
                newTree->setPosition(sf::Vector2f(centerX, centerY));

                m_buildings.push_back(newTree);

                // Haritada o kareyi dolu iþaretle (1: Duvar, 2: Aðaç vs. diyebilirsin ama þimdilik 1 kalsýn geçilmesin)
                m_level[i] = 1;
            }
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