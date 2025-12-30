#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "UIButton.h"
#include "UI/Ability.h" // Ability sýnýfýný tanýmasý için

class SelectedObjectPanel {
public:
    SelectedObjectPanel(float x, float y);

    void updateSelection(const std::string& name, int health, int maxHealth,
        sf::Texture* objectTexture,
        const std::vector<Ability>& abilities);

    void updateHealth(int health, int maxHealth);

    void updateQueue(const std::vector<sf::Texture*>& icons, float progress);

        void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);

    // --- EKSÝK OLAN FONKSÝYON ---
    void setPosition(float x, float y);

    // --- HATAYI ÇÖZEN KISIM ---
    void setVisible(bool status) { isVisible = status; }
    bool getVisible() const { return isVisible; }

    // Mouse kontrolünü cpp tarafýnda iptal etmiþtik ama 
    // fonksiyon imzasýný burada tutuyoruz ki HUD çaðýrdýðýnda hata vermesin.
    // Ýstersen tamamen silebilirsin ama HUD.cpp'deki çaðrýyý da silmen gerekir.
    bool isMouseOver(float mouseX, float mouseY) const;

private:
    sf::Font font;
    sf::Vector2f position;
    sf::RectangleShape panelBackground;
    sf::Sprite panelSprite;
    bool hasBackgroundTexture = false;

    // --- ÝÞTE EKSÝK OLAN DEÐÝÞKEN ---
    bool isVisible = false;

    // --- INFO PANEL ---
    sf::Text nameText;
    sf::Text hpText;
    sf::Sprite selectedIcon;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    // --- BUTTONS ---
    std::vector<UIButton> buttons;

    // Tooltip verisi
    std::vector<Ability> currentAbilities;

    // --- TOOLTIP ---
    sf::RectangleShape tooltipBackground;
    sf::Text tooltipText;
    bool showTooltip;

    void setupTooltip(const Ability& info);

    std::vector<sf::Texture*> productionIcons; // Kuyruktaki resimler
    float productionProgress = 0.0f;           // %0 ile %100 arasý
};