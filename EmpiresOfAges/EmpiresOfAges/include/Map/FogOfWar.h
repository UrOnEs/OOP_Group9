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

    void update(const std::vector<std::shared_ptr<class Entity>>& myEntities);
    void draw(sf::RenderWindow& window);

    bool isVisible(float x, float y) const;

    // --- YENÝ EKLENEN FONKSÝYON ---
    // Koordinattaki sis durumunu döndürür (Minimap için gerekli)
    FogState getFogAt(int x, int y) const;

private:
    int m_width;
    int m_height;
    int m_tileSize;

    std::vector<FogState> m_grid;
    sf::VertexArray m_vertices;

    void revealArea(int cx, int cy, float radius);
    void updateColor(int x, int y);
};