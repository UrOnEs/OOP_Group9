#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "ResourceBar.h"
#include "SelectedObjectPanel.h"
#include "UI/Ability.h"
#include "UI/Minimap.h" // <--- YENÝ: Minimap eklendi

class HUD {
public:
    HUD();

    // GÜNCELLEME: Init artýk harita bilgilerini de alýyor (Minimap kurulumu için)
    void init(int screenWidth, int screenHeight, int mapW, int mapH, int tileSize, const std::vector<int>& mapData);

    void draw(sf::RenderWindow& window);
    void update();

    // Seçili obje panelini güncelleme (Eski fonksiyonu koruyoruz)
    // Not: Artýk SelectedObjectPanel kendi içinde update fonksiyonuna sahip olabilir,
    // ancak uyumluluk için bu helper burada kalabilir.
    /* Eðer SelectedObjectPanel.h içinde public updateSelection varsa,
       direkt hud.selectedPanel.updateSelection(...) çaðýrabilirsiniz.
       Ben yine de eski yapýyý bozmamak için helper'ý kaldýrmýyorum ama
       kodda direkt selectedPanel'e eriþim veriyorum.
    */

    void handleEvent(const sf::Event& event);
    bool isMouseOverUI(const sf::Vector2i& mousePos) const;

    ResourceBar resourceBar;
    SelectedObjectPanel selectedPanel;
    Minimap minimap; // <--- YENÝ: Public eriþim (Game.cpp update edebilsin diye)

private:
    int m_width;
    int m_height;
};