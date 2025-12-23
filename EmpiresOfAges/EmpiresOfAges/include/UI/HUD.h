#pragma once
#include <SFML/Graphics.hpp>
#include <vector>             // std::vector için
#include "ResourceBar.h"
#include "SelectedObjectPanel.h"
#include "UI/Ability.h"     // <--- ARTIK BU DOSYAYI BULACAK

class HUD {
public:
    HUD();

    // Ekran boyutuna göre UI'ý kurar
    void init(int screenWidth, int screenHeight);

    void draw(sf::RenderWindow& window);
    void update();

    void updateSelectedObject(const std::string& name, int hp, int maxHp,
        sf::Texture* texture,
        const std::vector<Ability>& abilities);

    void handleEvent(const sf::Event& event);
    bool isMouseOverUI(const sf::Vector2i& mousePos) const;

    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;

private:
    int m_width;
    int m_height;
};