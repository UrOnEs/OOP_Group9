#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include "Map/FogOfWar.h"

class Minimap {
public:
    Minimap();

    // Harita verisini alýp görseli oluþturur (Sadece oyun baþýnda 1 kez çaðrýlýr)
    void init(int mapWidth, int mapHeight, int tileSize, const std::vector<int>& mapData, int screenWidth, int screenHeight);

    // Her frame güncellenir (Birimler ve Kamera)
    void update(const std::vector<std::shared_ptr<Entity>>& myUnits,
        const std::vector<std::shared_ptr<Entity>>& enemyUnits,
        const sf::View& currentView,
        FogOfWar* fog);

    void draw(sf::RenderWindow& window);

    // Týklama kontrolü (Týklandýysa true döner ve newCenter'ý doldurur)
    bool handleClick(const sf::Vector2i& mousePos, sf::Vector2f& outNewCenter);

    // Mouse minimap üzerinde mi? (HUD kontrolü için)
    bool isMouseOver(const sf::Vector2i& mousePos) const;

private:
    // Ayarlar
    sf::Vector2f m_size;      // Minimap'in ekrandaki boyutu (örn: 200x200 px)
    sf::Vector2f m_position;  // Ekrandaki konumu (Sað alt)
    float m_scaleX, m_scaleY; // Harita -> Minimap ölçek katsayýsý

    // Harita Boyutlarý
    int m_mapWidth;
    int m_mapHeight;
    int m_tileSize;

    // Görseller
    sf::RectangleShape m_background; // Çerçeve ve zemin
    sf::Texture m_mapTexture;        // Statik harita resmi (Piksel piksel oluþturacaðýz)
    sf::Sprite m_mapSprite;

    sf::RectangleShape m_cameraBox;  // Kameranýn baktýðý yer
    sf::VertexArray m_unitDots;      // Birim noktalarý (VertexArray daha performanslýdýr)

    // Savaþ Sisi Örtüsü
    sf::Image m_fogImage;
    sf::Texture m_fogTexture;
    sf::Sprite m_fogSprite;
    sf::Sprite m_frameSprite;
};