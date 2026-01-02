#pragma once
#include <SFML/Graphics.hpp>
#include "UI/UIButton.h"
#include <vector>

/**
 * @brief In-game settings menu overlay (Volume control, Quit).
 */
class SettingsMenuPanel {
public:
    SettingsMenuPanel();

    void init(float screenWidth, float screenHeight);
    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);

    void setVisible(bool status);
    bool isVisible() const;
    void setOnQuitGame(std::function<void()> action);

private:
    bool m_visible;
    sf::RectangleShape m_background; 
    sf::RectangleShape m_panel;      
    sf::Text m_titleText;
    sf::Font m_font;

    UIButton m_quitGameButton;
    std::function<void()> m_quitGameAction;
    UIButton m_closeButton;
    sf::Text m_musicLabel;      
    UIButton m_volDownButton;   
    UIButton m_volUpButton;     
    sf::Text m_volValueText;    
};