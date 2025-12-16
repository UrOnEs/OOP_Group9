#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "UIButton.h" // <-- UIButton sýnýfýný burada dahil ediyoruz

// AbilityInfo yapýsý
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

    // Seçili obje verilerini günceller
    void updateSelection(const std::string& name, int health, int maxHealth,
        sf::Texture* objectTexture,
        const std::vector<AbilityInfo>& abilities);

    // Eski handleInput yerine artýk handleEvent kullanýyoruz
    void handleEvent(const sf::Event& event);

    void draw(sf::RenderWindow& window);

private:
    sf::Font font;
    sf::Vector2f position;
    sf::RectangleShape panelBackground;

    // --- SAÐ PANEL (INFO) ---
    sf::Text nameText;
    sf::Text hpText;
    sf::Sprite selectedIcon;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    // --- SOL PANEL (ARTIK UIBUTTON KULLANIYOR) ---
    // Hatanýn asýl kaynaðý burasýydý, eski kodda burasý ActionButton struct'ýydý.
    std::vector<UIButton> buttons;
    std::vector<AbilityInfo> currentAbilities; // Tooltip göstermek için veriyi saklýyoruz

    // --- TOOLTIP (AÇIKLAMA KUTUSU) ---
    sf::RectangleShape tooltipBackground;
    sf::Text tooltipText;
    bool showTooltip;

    // Bu fonksiyon da private olarak tanýmlý olmalý
    void setupTooltip(const AbilityInfo& info);
};