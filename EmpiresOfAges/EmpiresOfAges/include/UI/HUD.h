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

    // Bu ikisini public yapt?m normalde private d? haberiniz olsun(Game.cpp k?sm?nda ihtiyac?m vard?(Sat?r 102))
    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;

private:

};
