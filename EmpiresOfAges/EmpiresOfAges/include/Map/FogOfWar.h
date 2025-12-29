#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Game/GameRules.h"

enum class FogState {
    Unexplored, // 0: Hiç keþfedilmemiþ (Tam Siyah)
    Explored,   // 1: Keþfedilmiþ ama þu an görülmüyor (Yarý Saydam Siyah)
    Visible     // 2: Þu an görüþ alanýnda (Þeffaf)
};

class FogOfWar {
public:
    FogOfWar(int width, int height, int tileSize);

    // Her frame'de çaðrýlýp sisi güncelleyecek
    void update(const std::vector<std::shared_ptr<class Entity>>& myEntities);

    // Çizim fonksiyonu
    void draw(sf::RenderWindow& window);

    // Bir nokta þu an görünür mü? (Düþmanlarý çizip çizmeme kararý için)
    bool isVisible(float x, float y) const;

private:
    int m_width;
    int m_height;
    int m_tileSize;

    std::vector<FogState> m_grid; // Mantýksal veri
    sf::VertexArray m_vertices;   // Görsel veri

    void revealArea(int cx, int cy, float radius);
    void updateColor(int x, int y);
};