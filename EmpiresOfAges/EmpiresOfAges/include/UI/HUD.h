#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceBar.h"
#include "SelectedObjectPanel.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window);
    void update();

private:
    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;
};

