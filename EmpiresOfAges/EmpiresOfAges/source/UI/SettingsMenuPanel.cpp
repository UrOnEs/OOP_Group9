#include "UI/SettingsMenuPanel.h"
#include "UI/AssetManager.h"
#include "Systems/SoundManager.h"

SettingsMenuPanel::SettingsMenuPanel() : m_visible(false) {
    // Font yükle (Hata kontrolü AssetManager içinde yapýlýyor varsayýyoruz)
    m_font = AssetManager::getFont("assets/fonts/Cinzel-VariableFont_wght.ttf");
}

void SettingsMenuPanel::init(float screenWidth, float screenHeight) {
    // 1. Karartma Arka Planý
    m_background.setSize(sf::Vector2f(screenWidth, screenHeight));
    m_background.setFillColor(sf::Color(0, 0, 0, 150)); // Yarý saydam siyah
    m_background.setPosition(0, 0);

    static sf::Texture panelTexture = AssetManager::getTexture("assets/ui/settingsMenu.png");

    // 2. Panel Gövdesi
    float panelW = 400.0f;
    float panelH = 500.0f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setTexture(&panelTexture); // Texture ata
    m_panel.setFillColor(sf::Color::White);
    m_panel.setOrigin(panelW / 2.0f, panelH / 2.0f);
    m_panel.setPosition(screenWidth / 2.0f, screenHeight / 2.0f);

    static sf::Texture settingsMenuButton = AssetManager::getTexture("assets/ui/settingsMenuButton.png");

    // 4. Kapat Butonu
    m_closeButton.setSize(120, 40);
    m_closeButton.setPosition((screenWidth / 2.0f) - 60, (screenHeight / 2.0f) + (panelH / 2.0f) - 100);
    m_closeButton.setTexture(settingsMenuButton);
    m_closeButton.setText("Kapat", m_font, 20,sf::Color(229, 212, 166));
    m_closeButton.setFillColor(sf::Color(180, 50, 50)); // Kýrmýzýmsý

    // Kapatma Callback'i
    m_closeButton.setCallback([this]() {
        this->setVisible(false);
        });

    // --- MÜZÝK SESÝ ETÝKETÝ ---
    m_musicLabel.setFont(m_font);
    m_musicLabel.setString("Music Volume");
    m_musicLabel.setCharacterSize(20);
    m_musicLabel.setFillColor(sf::Color(245, 228, 196));

    // Konum: Baþlýðýn biraz altýna
    float centerX = (screenWidth / 2.0f); // Biraz sola hizalý
    float centerY = (screenHeight / 2.0f) ;

    m_musicLabel.setPosition(centerX - 50, centerY - 80);

    float startY = centerY - 40; // Elemanlarýn Y konumu

    // 2. Eksi (-) Butonu
    m_volDownButton.setSize(40, 40);
    m_volDownButton.setPosition(centerX - 80, startY);
    m_volDownButton.setTexture(settingsMenuButton);
    m_volDownButton.setText("-", m_font, 30, sf::Color(245, 228, 196));
    // Butonun arka planýný þeffaf yapabilir veya kutu gibi býrakabilirsin
    m_volDownButton.setFillColor(sf::Color(70, 60, 50));

    // 3. Artý (+) Butonu
    m_volUpButton.setSize(40, 40);
    m_volUpButton.setPosition(centerX + 40, startY);
    m_volUpButton.setTexture(settingsMenuButton);
    m_volUpButton.setText("+", m_font, 30, sf::Color(245, 228, 196));
    m_volUpButton.setFillColor(sf::Color(70, 60, 50));

    // 4. Ortadaki Sayý (Ses Deðeri)
    m_volValueText.setFont(m_font);
    m_volValueText.setCharacterSize(24);
    m_volValueText.setFillColor(sf::Color::White);
    // Baþlangýç deðerini al ve yaz
    int currentVol = (int)SoundManager::getMusicVolume();
    m_volValueText.setString(std::to_string(currentVol));

    // Sayýyý ortala
    sf::FloatRect textRect = m_volValueText.getLocalBounds();
    m_volValueText.setOrigin(textRect.width / 2.0f, textRect.height / 2.0f);
    m_volValueText.setPosition(centerX, startY + 20); // Butonlarýn ortasýna

    // --- CALLBACKLER (Týklama Olaylarý) ---

    // Eksiye basýnca
    m_volDownButton.setCallback([this]() {
        float vol = SoundManager::getMusicVolume();
        vol -= 10.0f; // 10 azalt
        if (vol < 0) vol = 0;

        SoundManager::setMusicVolume(vol);

        // Yazýyý güncelle
        this->m_volValueText.setString(std::to_string((int)vol));
        });

    // Artýya basýnca
    m_volUpButton.setCallback([this]() {
        float vol = SoundManager::getMusicVolume();
        vol += 10.0f; // 10 artýr
        if (vol > 100) vol = 100;

        SoundManager::setMusicVolume(vol);

        // Yazýyý güncelle
        this->m_volValueText.setString(std::to_string((int)vol));
        });
}

void SettingsMenuPanel::handleEvent(const sf::Event& event) {
    if (!m_visible) return;

    m_volDownButton.handleEvent(event);
    m_volUpButton.handleEvent(event);
    m_closeButton.handleEvent(event);
}

void SettingsMenuPanel::draw(sf::RenderWindow& window) {
    if (!m_visible) return;

    window.draw(m_background); // Arkadaki oyunu karart
    window.draw(m_panel);      // Kutuyu çiz
    window.draw(m_titleText);  // Baþlýðý çiz
    m_closeButton.draw(window);
    window.draw(m_musicLabel);
    m_volDownButton.draw(window);
    m_volUpButton.draw(window);
    window.draw(m_volValueText);
}

void SettingsMenuPanel::setVisible(bool status) {
    m_visible = status;
}

bool SettingsMenuPanel::isVisible() const {
    return m_visible;
}