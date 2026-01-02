#include "UI/SettingsMenuPanel.h"
#include "UI/AssetManager.h"
#include "Systems/SoundManager.h"

SettingsMenuPanel::SettingsMenuPanel() : m_visible(false) {
    m_font = AssetManager::getFont("assets/fonts/Cinzel-VariableFont_wght.ttf");
}

void SettingsMenuPanel::setOnQuitGame(std::function<void()> action) {
    m_quitGameAction = action;
}

void SettingsMenuPanel::init(float screenWidth, float screenHeight) {
    m_background.setSize(sf::Vector2f(screenWidth, screenHeight));
    m_background.setFillColor(sf::Color(0, 0, 0, 150));
    m_background.setPosition(0, 0);

    static sf::Texture panelTexture = AssetManager::getTexture("assets/ui/settingsMenu.png");

    float panelW = 400.0f;
    float panelH = 500.0f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setTexture(&panelTexture);
    m_panel.setFillColor(sf::Color::White);
    m_panel.setOrigin(panelW / 2.0f, panelH / 2.0f);
    m_panel.setPosition(screenWidth / 2.0f, screenHeight / 2.0f);

    static sf::Texture settingsMenuButton = AssetManager::getTexture("assets/ui/settingsMenuButton.png");

    // Music Volume Label
    m_musicLabel.setFont(m_font);
    m_musicLabel.setString("Music Volume");
    m_musicLabel.setCharacterSize(20);
    m_musicLabel.setFillColor(sf::Color(245, 228, 196));

    float centerX = (screenWidth / 2.0f) - 70.0f;
    float centerY = (screenHeight / 2.0f) - 200.0f;
    m_musicLabel.setPosition(centerX, centerY);

    float startY = centerY + 40;

    // (-) Button
    m_volDownButton.setSize(40, 40);
    m_volDownButton.setPosition(centerX - 40, startY);
    m_volDownButton.setTexture(settingsMenuButton);
    m_volDownButton.setText("-", m_font, 30, sf::Color(245, 228, 196));
    m_volDownButton.setFillColor(sf::Color(70, 60, 50));

    // (+) Button
    m_volUpButton.setSize(40, 40);
    m_volUpButton.setPosition(centerX + 160, startY);
    m_volUpButton.setTexture(settingsMenuButton);
    m_volUpButton.setText("+", m_font, 30, sf::Color(245, 228, 196));
    m_volUpButton.setFillColor(sf::Color(70, 60, 50));

    // Value Text
    m_volValueText.setFont(m_font);
    m_volValueText.setCharacterSize(24);
    m_volValueText.setFillColor(sf::Color::White);

    int currentVol = (int)SoundManager::getMusicVolume();
    m_volValueText.setString(std::to_string(currentVol));

    sf::FloatRect textRect = m_volValueText.getLocalBounds();
    m_volValueText.setOrigin(textRect.width / 2.0f, textRect.height / 2.0f);
    m_volValueText.setPosition(centerX + 70, startY + 15);

    // Callbacks
    m_volDownButton.setCallback([this]() {
        int vol = (int)std::round(SoundManager::getMusicVolume());
        vol -= 10;
        if (vol < 0) vol = 0;

        SoundManager::setMusicVolume((float)vol);
        this->m_volValueText.setString(std::to_string(vol));

        sf::FloatRect tr = this->m_volValueText.getLocalBounds();
        this->m_volValueText.setOrigin(tr.width / 2.0f, tr.height / 2.0f);
        });

    m_volUpButton.setCallback([this]() {
        int vol = (int)std::round(SoundManager::getMusicVolume());
        vol += 10;
        if (vol > 100) vol = 100;

        SoundManager::setMusicVolume((float)vol);
        this->m_volValueText.setString(std::to_string(vol));

        sf::FloatRect tr = this->m_volValueText.getLocalBounds();
        this->m_volValueText.setOrigin(tr.width / 2.0f, tr.height / 2.0f);
        });

    // Quit Game Button
    m_quitGameButton.setSize(140, 40);
    m_quitGameButton.setPosition((screenWidth / 2.0f) - 60, (screenHeight / 2.0f) + (panelH / 2.0f) - 100);
    m_quitGameButton.setTexture(settingsMenuButton);
    m_quitGameButton.setText("Quit Game", m_font, 20, sf::Color(229, 212, 166));

    m_quitGameButton.setCallback([this]() {
        if (m_quitGameAction) {
            m_quitGameAction();
        }
        });

    // Return Button
    m_closeButton.setSize(140, 40);
    m_closeButton.setPosition((screenWidth / 2.0f) - 60, (screenHeight / 2.0f) + (panelH / 2.0f) - 150);
    m_closeButton.setTexture(settingsMenuButton);
    m_closeButton.setText("Return", m_font, 20, sf::Color(229, 212, 166));

    m_closeButton.setCallback([this]() {
        this->setVisible(false);
        });
}

void SettingsMenuPanel::handleEvent(const sf::Event& event) {
    if (!m_visible) return;

    m_volDownButton.handleEvent(event);
    m_volUpButton.handleEvent(event);
    m_closeButton.handleEvent(event);
    m_quitGameButton.handleEvent(event);
}

void SettingsMenuPanel::draw(sf::RenderWindow& window) {
    if (!m_visible) return;

    window.draw(m_background);
    window.draw(m_panel);
    window.draw(m_titleText);
    m_closeButton.draw(window);
    window.draw(m_musicLabel);
    m_volDownButton.draw(window);
    m_volUpButton.draw(window);
    window.draw(m_volValueText);
    m_quitGameButton.draw(window);
}

void SettingsMenuPanel::setVisible(bool status) {
    m_visible = status;

    if (m_visible) {
        int currentVol = (int)std::round(SoundManager::getMusicVolume());
        m_volValueText.setString(std::to_string(currentVol));

        sf::FloatRect tr = m_volValueText.getLocalBounds();
        m_volValueText.setOrigin(tr.width / 2.0f, tr.height / 2.0f);
    }
}

bool SettingsMenuPanel::isVisible() const {
    return m_visible;
}