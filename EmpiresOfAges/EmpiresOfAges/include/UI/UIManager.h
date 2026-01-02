#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "UIPanel.h"

class UIManager {
public:
    void addPanel(const UIPanel& panel);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);

private:
    std::vector<UIPanel> panels;
};