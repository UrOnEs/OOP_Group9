#include "UI/HUD.h"

HUD::HUD()
    : selectedPanel(20.0f, 600.0f)
{
}

void HUD::update() {
    // RTS resource update vs.
}

// --- BU FONKS?YONU DE???T?R?YORUZ ---
void HUD::handleEvent(const sf::Event& event) {
    // Olay? panele ilet (O da butonlara iletecek)
    selectedPanel.handleEvent(event);
}

void HUD::draw(sf::RenderWindow& window) {
    resourceBar.draw(window);
    selectedPanel.draw(window);
}