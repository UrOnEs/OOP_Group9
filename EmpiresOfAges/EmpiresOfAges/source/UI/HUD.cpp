#include "UI/HUD.h"

HUD::HUD() {}

void HUD::update() {
    // RTS için ileride resource veya selection update burada olur
}

void HUD::draw(sf::RenderWindow& window) {
    resourceBar.draw(window);
    selectedPanel.draw(window);
}
