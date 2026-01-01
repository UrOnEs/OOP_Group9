#pragma once
#include <SFML/Graphics.hpp>
#include "UI/UIButton.h"
#include <vector>

class SettingsMenuPanel {
public:
    SettingsMenuPanel();

    // Ekran boyutuna göre paneli ortalar
    void init(float screenWidth, float screenHeight);

    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);

    void setVisible(bool status);
    bool isVisible() const;

private:
    bool m_visible;
    sf::RectangleShape m_background; // Yarý saydam siyah arka plan (tüm ekran)
    sf::RectangleShape m_panel;      // Menünün kendisi
    sf::Text m_titleText;
    sf::Font m_font;

    UIButton m_closeButton;
    sf::Text m_musicLabel;      // "Müzik Sesi" yazýsý
    UIButton m_volDownButton;   // (-) Butonu
    UIButton m_volUpButton;     // (+) Butonu
    sf::Text m_volValueText;    // Ortada yazacak sayý (örn: "50")
};