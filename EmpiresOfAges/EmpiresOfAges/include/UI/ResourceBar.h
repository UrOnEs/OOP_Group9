#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class ResourceBar {
public:
    ResourceBar();
    void updateResources(int wood, int food, int gold, int stone);
    void draw(sf::RenderWindow& window);

private:
    sf::Font font;

    sf::RectangleShape backgroundBar;

    sf::Text woodText;
    sf::Text foodText;
    sf::Text goldText;
    sf::Text stoneText;
};