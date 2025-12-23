#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "UIButton.h" // <-- UIButton s?n?f?n? burada dahil ediyoruz

// AbilityInfo yap?s?
struct AbilityInfo {
    int id;
    std::string name;
    std::string costText;
    std::string description;
    sf::Texture* iconTexture;
};

class SelectedObjectPanel {
public:
    SelectedObjectPanel(float x, float y);

    // Se?ili obje verilerini g?nceller
    void updateSelection(const std::string& name, int health, int maxHealth,
        sf::Texture* objectTexture,
        const std::vector<AbilityInfo>& abilities);

    // Eski handleInput yerine art?k handleEvent kullan?yoruz
    void handleEvent(const sf::Event& event);

    void draw(sf::RenderWindow& window);

private:
    sf::Font font;
    sf::Vector2f position;
    sf::RectangleShape panelBackground;

    // --- SA? PANEL (INFO) ---
    sf::Text nameText;
    sf::Text hpText;
    sf::Sprite selectedIcon;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    // --- SOL PANEL (ARTIK UIBUTTON KULLANIYOR) ---
    // Hatan?n as?l kayna?? buras?yd?, eski kodda buras? ActionButton struct'?yd?.
    std::vector<UIButton> buttons;
    std::vector<AbilityInfo> currentAbilities; // Tooltip g?stermek i?in veriyi sakl?yoruz

    // --- TOOLTIP (A?IKLAMA KUTUSU) ---
    sf::RectangleShape tooltipBackground;
    sf::Text tooltipText;
    bool showTooltip;

    // Bu fonksiyon da private olarak tan?ml? olmal?
    void setupTooltip(const AbilityInfo& info);
};