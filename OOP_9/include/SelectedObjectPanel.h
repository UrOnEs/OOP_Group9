#pragma once
#include <SFML/Graphics.hpp>

class SelectedObjectPanel {
public:
    SelectedObjectPanel();
    void updateSelection(const std::string& name, int health);
    void draw(sf::RenderWindow& window);

private:
    sf::Font font;
    sf::Text nameText;
    sf::Text healthText;
};