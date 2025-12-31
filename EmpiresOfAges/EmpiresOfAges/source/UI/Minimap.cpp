#include "UI/Minimap.h"
#include "Game/GameRules.h"
#include <iostream>

Minimap::Minimap() {
    m_size = sf::Vector2f(200.f, 200.f);

    // Arkaplan
    m_background.setSize(m_size);
    m_background.setFillColor(sf::Color(20, 20, 20));
    m_background.setOutlineThickness(2);
    m_background.setOutlineColor(sf::Color(200, 150, 50));

    // Kamera kutusu
    m_cameraBox.setFillColor(sf::Color::Transparent);
    m_cameraBox.setOutlineThickness(1.0f);
    m_cameraBox.setOutlineColor(sf::Color::White);

    m_unitDots.setPrimitiveType(sf::Quads);
}

void Minimap::init(int mapWidth, int mapHeight, int tileSize, const std::vector<int>& mapData, int screenWidth, int screenHeight) {
    m_mapWidth = mapWidth;
    m_mapHeight = mapHeight;
    m_tileSize = tileSize;

    // Konum (Sað Alt)
    m_position = sf::Vector2f(screenWidth - m_size.x - 10, screenHeight - m_size.y - 10);
    m_background.setPosition(m_position);
    m_mapSprite.setPosition(m_position);
    m_fogSprite.setPosition(m_position); // Fog Sprite konumu

    float worldWidth = (float)(mapWidth * tileSize);
    float worldHeight = (float)(mapHeight * tileSize);
    m_scaleX = m_size.x / worldWidth;
    m_scaleY = m_size.y / worldHeight;

    // --- HARÝTA DOKUSU ---
    sf::Image mapImg;
    mapImg.create(mapWidth, mapHeight, sf::Color::Black);

    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            int tileType = mapData[x + y * mapWidth];
            sf::Color color;
            if (tileType == 0) color = sf::Color(34, 139, 34); // Çimen
            else color = sf::Color(100, 100, 100);             // Duvar
            mapImg.setPixel(x, y, color);
        }
    }

    m_mapTexture.loadFromImage(mapImg);
    m_mapSprite.setTexture(m_mapTexture);
    m_mapSprite.setScale(m_size.x / (float)mapWidth, m_size.y / (float)mapHeight);

    // --- FOG OF WAR GÖRSELÝ HAZIRLIÐI ---
    m_fogImage.create(mapWidth, mapHeight, sf::Color::Black); // Baþlangýçta simsiyah
    m_fogTexture.loadFromImage(m_fogImage);
    m_fogSprite.setTexture(m_fogTexture);
    m_fogSprite.setScale(m_mapSprite.getScale()); // Harita ile ayný ölçekte
}

void Minimap::update(const std::vector<std::shared_ptr<Entity>>& myUnits,
    const std::vector<std::shared_ptr<Entity>>& enemyUnits,
    const sf::View& currentView,
    FogOfWar* fog)
{
    // 1. KAMERA KUTUSU
    sf::Vector2f viewSize = currentView.getSize();
    sf::Vector2f viewCenter = currentView.getCenter();
    sf::Vector2f viewTopLeft = viewCenter - (viewSize / 2.0f);

    float boxW = viewSize.x * m_scaleX;
    float boxH = viewSize.y * m_scaleY;
    float boxX = m_position.x + (viewTopLeft.x * m_scaleX);
    float boxY = m_position.y + (viewTopLeft.y * m_scaleY);

    if (boxX < m_position.x) boxX = m_position.x;
    if (boxY < m_position.y) boxY = m_position.y;
    if (boxX + boxW > m_position.x + m_size.x) boxX = (m_position.x + m_size.x) - boxW;
    if (boxY + boxH > m_position.y + m_size.y) boxY = (m_position.y + m_size.y) - boxH;

    m_cameraBox.setSize(sf::Vector2f(boxW, boxH));
    m_cameraBox.setPosition(boxX, boxY);

    // 2. FOG OF WAR GÜNCELLEMESÝ (YENÝ KOD)
    if (fog) {
        // Haritadaki her karo için sis durumunu kontrol edip pixel rengini ayarla
        for (int x = 0; x < m_mapWidth; ++x) {
            for (int y = 0; y < m_mapHeight; ++y) {
                FogState state = fog->getFogAt(x, y);
                sf::Color c = sf::Color::Black; // Varsayýlan: Unexplored

                if (state == FogState::Visible) {
                    c = sf::Color::Transparent; // Açýk
                }
                else if (state == FogState::Explored) {
                    c = sf::Color(0, 0, 0, 150); // Yarý saydam siyah
                }
                // Unexplored ise zaten Black kalýr.

                m_fogImage.setPixel(x, y, c);
            }
        }
        // Texture'ý RAM'den GPU'ya gönder
        m_fogTexture.update(m_fogImage);
    }

    // 3. BÝRÝMLER
    m_unitDots.clear();

    auto addDot = [&](const std::shared_ptr<Entity>& e, sf::Color color) {
        sf::Vector2f wPos = e->getPosition();
        float mx = m_position.x + (wPos.x * m_scaleX);
        float my = m_position.y + (wPos.y * m_scaleY);
        float dotSize = 3.0f;

        m_unitDots.append(sf::Vertex(sf::Vector2f(mx, my), color));
        m_unitDots.append(sf::Vertex(sf::Vector2f(mx + dotSize, my), color));
        m_unitDots.append(sf::Vertex(sf::Vector2f(mx + dotSize, my + dotSize), color));
        m_unitDots.append(sf::Vertex(sf::Vector2f(mx, my + dotSize), color));
        };

    // Dostlar
    for (const auto& u : myUnits) {
        if (u->getIsAlive()) addDot(u, sf::Color::Cyan);
    }

    // Düþmanlar (Sadece görünürse)
    for (const auto& u : enemyUnits) {
        if (u->getIsAlive()) {
            bool visible = true;
            if (fog) visible = fog->isVisible(u->getPosition().x, u->getPosition().y);

            if (visible) addDot(u, sf::Color::Red);
        }
    }
}

void Minimap::draw(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_mapSprite);  // 1. Zemin
    window.draw(m_fogSprite);  // 2. Sis (Zemini karartýr)
    window.draw(m_unitDots);   // 3. Birimler (Sisin üstünde parlak görünsün)
    window.draw(m_cameraBox);  // 4. Kamera Çerçevesi
}

bool Minimap::handleClick(const sf::Vector2i& mousePos, sf::Vector2f& outNewCenter) {
    if (!isMouseOver(mousePos)) return false;

    float relX = mousePos.x - m_position.x;
    float relY = mousePos.y - m_position.y;

    float worldX = relX / m_scaleX;
    float worldY = relY / m_scaleY;

    outNewCenter = sf::Vector2f(worldX, worldY);
    return true;
}

bool Minimap::isMouseOver(const sf::Vector2i& mousePos) const {
    return m_background.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y);
}