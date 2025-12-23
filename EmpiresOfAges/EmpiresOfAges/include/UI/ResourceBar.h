#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Game/Player.h"

class ResourceBar {
public:
    ResourceBar();
    void updateResources(int wood, int food, int gold, int stone,Player player);
    void draw(sf::RenderWindow& window);

    // YENÝ: Geniþliði ayarlamak için
    void setWidth(float width);
    float getHeight() const { return 40.f; } // Sabit yükseklik getter

private:
    sf::Font font;
    sf::RectangleShape backgroundBar;

    sf::Text woodText;
    sf::Text foodText;
    sf::Text goldText;
    sf::Text stoneText;
    sf::Text populationText;
};