#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceBar.h"
#include "SelectedObjectPanel.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window);
    void update();
    
    
    // Bu ikisini public yaptým normalde private dý haberiniz olsun(Game.cpp kýsmýnda ihtiyacým vardý(Satýr 102))
    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;

private:

};

