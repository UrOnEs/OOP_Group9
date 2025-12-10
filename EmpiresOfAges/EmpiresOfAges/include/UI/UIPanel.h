#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "UIButton.h"

class UIPanel {
public:
    UIPanel(const sf::Vector2f& size, const sf::Vector2f& position);

    void addButton(const UIButton& button);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);

private:
    sf::RectangleShape background;
    std::vector<UIButton> buttons;
};


