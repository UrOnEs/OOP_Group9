#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceBar.h"
#include "SelectedObjectPanel.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window);
    void update();
    void updateSelectedObject(const std::string& name, int hp, int maxHp,
        sf::Texture* texture,
        const std::vector<AbilityInfo>& abilities);
    void handleEvent(const sf::Event& event);
    
    // Bu ikisini public yaptým normalde private dý haberiniz olsun(Game.cpp kýsmýnda ihtiyacým vardý(Satýr 102))
    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;

private:

};

