#include "UI/HUD.h"

HUD::HUD()
    : selectedPanel(0.f, 0.f), m_width(800), m_height(600)
{
}

void HUD::init(int screenWidth, int screenHeight, int mapW, int mapH, int tileSize, const std::vector<int>& mapData) {
    m_width = screenWidth;
    m_height = screenHeight;

    resourceBar.setWidth((float)screenWidth);
    resourceBar.setSettingsCallback([this]() {
        this->settingsPanel.setVisible(true);
        });

    float panelHeight = 140.0f;
    float panelY = (float)screenHeight - panelHeight;
    selectedPanel.setPosition(0.f, panelY);

    minimap.init(mapW, mapH, tileSize, mapData, screenWidth, screenHeight);
    settingsPanel.init((float)screenWidth, (float)screenHeight);
}

void HUD::update() {
    // General HUD updates if needed.
}

void HUD::handleEvent(const sf::Event& event) {
    resourceBar.handleEvent(event);
    settingsPanel.handleEvent(event);

    if (!settingsPanel.isVisible()) {
        selectedPanel.handleEvent(event);
    }
}

void HUD::draw(sf::RenderWindow& window) {
    resourceBar.draw(window);
    selectedPanel.draw(window);
    minimap.draw(window);
    settingsPanel.draw(window);
}

bool HUD::isMouseOverUI(const sf::Vector2i& mousePos) const {
    if (settingsPanel.isVisible()) return true;

    if (mousePos.y < (int)resourceBar.getHeight()) {
        return true;
    }

    if (selectedPanel.isMouseOver((float)mousePos.x, (float)mousePos.y)) {
        return true;
    }

    if (minimap.isMouseOver(mousePos)) {
        return true;
    }

    return false;
}