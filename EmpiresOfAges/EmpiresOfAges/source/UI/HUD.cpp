#include "UI/HUD.h"

HUD::HUD()
    : selectedPanel(0.f, 0.f), m_width(800), m_height(600)
{
}

// GÜNCELLEME: Parametreler geniþletildi
void HUD::init(int screenWidth, int screenHeight, int mapW, int mapH, int tileSize, const std::vector<int>& mapData) {
    m_width = screenWidth;
    m_height = screenHeight;

    // 1. ResourceBar
    resourceBar.setWidth((float)screenWidth);

    // 2. SelectedObjectPanel
    float panelHeight = 140.0f;
    float panelY = (float)screenHeight - panelHeight;
    selectedPanel.setPosition(0.f, panelY);

    // 3. MINIMAP BAÞLAT (YENÝ)
    // Minimap'i harita verileriyle kuruyoruz
    minimap.init(mapW, mapH, tileSize, mapData, screenWidth, screenHeight);
}

void HUD::update() {
    // HUD içi genel animasyonlar varsa buraya eklenir.
    // Not: Minimap'in birimleri güncellemesi (update) iþlemi 
    // Game.cpp içinde entity listeleri verilerek yapýlýyor.
}

void HUD::handleEvent(const sf::Event& event) {
    selectedPanel.handleEvent(event);
}

void HUD::draw(sf::RenderWindow& window) {
    resourceBar.draw(window);
    selectedPanel.draw(window);
    minimap.draw(window); // <--- YENÝ: Minimap çizimi
}

bool HUD::isMouseOverUI(const sf::Vector2i& mousePos) const {
    // 1. Üst Kaynak Barý
    if (mousePos.y < (int)resourceBar.getHeight()) {
        return true;
    }

    // 2. SelectedObjectPanel
    if (selectedPanel.isMouseOver((float)mousePos.x, (float)mousePos.y)) {
        return true;
    }

    // 3. Minimap (YENÝ)
    // Eðer mouse minimap üzerindeyse true dön, böylece oyun dünyasýna yanlýþlýkla týklanmaz.
    if (minimap.isMouseOver(mousePos)) {
        return true;
    }

    return false;
}