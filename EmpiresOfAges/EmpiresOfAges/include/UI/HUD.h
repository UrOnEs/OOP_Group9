#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "ResourceBar.h"
#include "SelectedObjectPanel.h"
#include "UI/Ability.h"
#include "UI/Minimap.h"
#include "UI/SettingsMenuPanel.h"

/**
 * @brief Main Heads-Up Display manager.
 * Orchestrates the ResourceBar, Minimap, SelectedObjectPanel, and SettingsMenu.
 */
class HUD {
public:
    HUD();

    void init(int screenWidth, int screenHeight, int mapW, int mapH, int tileSize, const std::vector<int>& mapData);
    void draw(sf::RenderWindow& window);
    void update();
    void handleEvent(const sf::Event& event);
    bool isMouseOverUI(const sf::Vector2i& mousePos) const;

    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;
    Minimap minimap;
    SettingsMenuPanel settingsPanel;

private:
    int m_width;
    int m_height;
};