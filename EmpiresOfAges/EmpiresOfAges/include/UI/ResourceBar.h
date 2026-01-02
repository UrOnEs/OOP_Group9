#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Game/Player.h"
#include "UI/UIButton.h"

/**
 * @brief Top HUD panel displaying player resources and population.
 */
class ResourceBar {
public:
    ResourceBar();
    void updateResources(int wood, int food, int gold, int stone, Player player);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);

    void setSettingsCallback(std::function<void()> cb);
    void setWidth(float width);
    float getHeight() const { return 40.f; }

private:
    sf::Font font;
    sf::RectangleShape backgroundBar;

    sf::Sprite barSprite;
    bool hasTexture = false;

    sf::Sprite iconWood, iconFood, iconGold, iconStone;

    sf::Text woodText;
    sf::Text foodText;
    sf::Text goldText;
    sf::Text stoneText;
    sf::Text populationText;

    UIButton settingsButton;
};