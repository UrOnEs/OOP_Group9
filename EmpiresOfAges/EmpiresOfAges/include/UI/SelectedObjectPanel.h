#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "UIButton.h"
#include "UI/Ability.h"

/**
 * @brief Bottom UI panel showing details and actions for the selected entity.
 */
class SelectedObjectPanel {
public:
    SelectedObjectPanel(float x, float y);

    /**
     * @brief Updates the panel with the selected entity's data and abilities.
     */
    void updateSelection(const std::string& name, int health, int maxHealth,
        sf::Texture* objectTexture,
        const std::vector<Ability>& abilities);

    void updateHealth(int health, int maxHealth);
    void updateQueue(const std::vector<sf::Texture*>& icons, float progress);

    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);
    void setPosition(float x, float y);

    void setVisible(bool status) { isVisible = status; }
    bool getVisible() const { return isVisible; }
    bool isMouseOver(float mouseX, float mouseY) const;

private:
    sf::Font font;
    sf::Vector2f position;
    sf::RectangleShape panelBackground;
    sf::Sprite panelSprite;
    bool hasBackgroundTexture = false;
    bool isVisible = false;

    // Info Section
    sf::Text nameText;
    sf::Text hpText;
    sf::Sprite selectedIcon;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    // Actions
    std::vector<UIButton> buttons;
    std::vector<Ability> currentAbilities;

    // Tooltip
    sf::RectangleShape tooltipBackground;
    sf::Text tooltipText;
    bool showTooltip;
    void setupTooltip(const Ability& info);

    // Production Queue
    std::vector<sf::Texture*> productionIcons;
    float productionProgress = 0.0f;
};