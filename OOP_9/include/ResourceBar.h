#pragma once
#include <SFML/Graphics.hpp>

class ResourceBar {
public:
    ResourceBar();
    void updateResources(int wood, int food, int gold, int stone);
    void draw(sf::RenderWindow& window);

private:
    sf::Font font;
    sf::Text woodText, foodText, goldText, stoneText;
};
