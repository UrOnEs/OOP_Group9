#include "Map/FogOfWar.h"
#include "Entity System/Entity.h"
#include "Entity System/Entity Type/Building.h"
#include "Entity System/Entity Type/TownCenter.h"
#include <cmath>

FogOfWar::FogOfWar(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize)
{
    // Grid'i baþlat (Hepsi Unexplored)
    m_grid.resize(width * height, FogState::Unexplored);

    // Vertex Array'i hazýrla (Quads)
    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize(width * height * 4);

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            sf::Vertex* quad = &m_vertices[(x + y * width) * 4];

            float px = (float)x * tileSize;
            float py = (float)y * tileSize;

            quad[0].position = sf::Vector2f(px, py);
            quad[1].position = sf::Vector2f(px + tileSize, py);
            quad[2].position = sf::Vector2f(px + tileSize, py + tileSize);
            quad[3].position = sf::Vector2f(px, py + tileSize);

            // Baþlangýç rengi: Simsiyah
            for (int i = 0; i < 4; ++i) quad[i].color = sf::Color::Black;
        }
    }
}

void FogOfWar::update(const std::vector<std::shared_ptr<Entity>>& myEntities) {
    // 1. ADIM: Önceki "Visible" olanlarý "Explored" durumuna düþür
    // (Unexplored olanlar Unexplored kalmaya devam eder)
    for (int i = 0; i < m_width * m_height; ++i) {
        if (m_grid[i] == FogState::Visible) {
            m_grid[i] = FogState::Explored;
            // Rengi güncelle (Yarý saydam siyah)
            int x = i % m_width;
            int y = i / m_width;
            updateColor(x, y);
        }
    }

    // 2. ADIM: Birimlerin olduðu yerleri "Visible" yap
    for (const auto& entity : myEntities) {
        if (!entity->getIsAlive()) continue;

        sf::Vector2f pos = entity->getPosition();
        int cx = static_cast<int>(pos.x / m_tileSize);
        int cy = static_cast<int>(pos.y / m_tileSize);

        // --- GÜNCELLENEN KISIM: TÝPE GÖRE MENZÝL SEÇÝMÝ ---
        float sightRange = GameRules::Range_Sight_Unit; // Varsayýlan (Asker/Köylü)

        // Eðer bu bir BÝNA ise
        if (std::dynamic_pointer_cast<Building>(entity)) {
            // Eðer Ana Bina (TownCenter) ise daha da geniþ görsün
            if (std::dynamic_pointer_cast<TownCenter>(entity)) {
                sightRange = GameRules::Range_Sight_TownCenter;
            }
            else {
                sightRange = GameRules::Range_Sight_Building;
            }
        }
        // ----------------------------------------------------

        revealArea(cx, cy, sightRange);
    }
}

void FogOfWar::revealArea(int cx, int cy, float radius) {
    int rGrid = static_cast<int>(radius / m_tileSize);
    int rSq = rGrid * rGrid;

    for (int y = cy - rGrid; y <= cy + rGrid; ++y) {
        for (int x = cx - rGrid; x <= cx + rGrid; ++x) {
            if (x < 0 || y < 0 || x >= m_width || y >= m_height) continue;

            // Dairesel görüþ kontrolü
            int dx = x - cx;
            int dy = y - cy;
            if (dx * dx + dy * dy <= rSq) {
                int index = x + y * m_width;
                m_grid[index] = FogState::Visible;
                updateColor(x, y);
            }
        }
    }
}

void FogOfWar::updateColor(int x, int y) {
    int index = x + y * m_width;
    sf::Vertex* quad = &m_vertices[index * 4];
    sf::Color color;

    switch (m_grid[index]) {
    case FogState::Unexplored:
        color = sf::Color::Black; // Tamamen görünmez
        break;
    case FogState::Explored:
        color = sf::Color(0, 0, 0, 150); // Karartýlmýþ (Daha önce görülmüþ)
        break;
    case FogState::Visible:
        color = sf::Color::Transparent; // Tamamen açýk
        break;
    }

    for (int i = 0; i < 4; ++i) quad[i].color = color;
}

void FogOfWar::draw(sf::RenderWindow& window) {
    window.draw(m_vertices);
}

bool FogOfWar::isVisible(float x, float y) const {
    int gx = static_cast<int>(x / m_tileSize);
    int gy = static_cast<int>(y / m_tileSize);

    if (gx < 0 || gy < 0 || gx >= m_width || gy >= m_height) return false;

    return m_grid[gx + gy * m_width] == FogState::Visible;
}