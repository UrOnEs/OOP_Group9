#include "UI/HUD.h"

// Constructor'da panele geçici deðer veriyoruz, init ile düzelteceðiz.
HUD::HUD()
    : selectedPanel(0.f, 0.f), m_width(800), m_height(600)
{
}

void HUD::init(int screenWidth, int screenHeight) {
    m_width = screenWidth;
    m_height = screenHeight;

    // 1. ResourceBar'ý ekran geniþliðine ayarla
    resourceBar.setWidth((float)screenWidth);

    // 2. SelectedObjectPanel'i sol alta sabitle
    // Panel yüksekliði 140px (SelectedObjectPanel.cpp'de tanýmlý)
    float panelHeight = 140.0f;
    float panelY = (float)screenHeight - panelHeight;

    // Panel sýnýfýna setPosition eklememiz gerekmiþti, hemen aþaðýda ekleyeceðiz.
    selectedPanel.setPosition(0.f, panelY);
}

void HUD::update() {
    // RTS resource update vs.
}

void HUD::handleEvent(const sf::Event& event) {
    selectedPanel.handleEvent(event);
}

void HUD::draw(sf::RenderWindow& window) {
    resourceBar.draw(window);
    selectedPanel.draw(window);
}

// --- DÜZELTÝLMÝÞ MOUSE KORUMASI ---
bool HUD::isMouseOverUI(const sf::Vector2i& mousePos) const {
    // 1. Üst Kaynak Barý Kontrolü (Dinamik yükseklik)
    if (mousePos.y < (int)resourceBar.getHeight()) {
        return true;
    }

    // 2. SelectedObjectPanel Kontrolü
    // Artýk panelin kendi isMouseOver fonksiyonu "Görünür olup olmadýðýný" da kontrol ediyor.
    if (selectedPanel.isMouseOver((float)mousePos.x, (float)mousePos.y)) {
        return true;
    }

    return false;
}