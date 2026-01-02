#include "UI/ResourceBar.h"
#include "UI/AssetManager.h"

ResourceBar::ResourceBar() {
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Handle font loading error
    }

    backgroundBar.setSize(sf::Vector2f(1920.f, 40.f));
    backgroundBar.setFillColor(sf::Color(0, 0, 0, 150));
    backgroundBar.setPosition(0, 0);

    sf::Texture& barTex = AssetManager::getTexture("assets/ui/resourcePanel_b.png");
    if (barTex.getSize().x > 0) {
        barTex.setRepeated(true);
        barSprite.setTexture(barTex);
        hasTexture = true;
        barSprite.setPosition(0, 0);
    }

    auto setupIcon = [&](sf::Sprite& sprite, const std::string& path) {
        sf::Texture& tex = AssetManager::getTexture(path);
        if (tex.getSize().x > 0) {
            sprite.setTexture(tex);
            float iconSize = 32.0f;
            sprite.setScale(iconSize / tex.getSize().x, iconSize / tex.getSize().y);
        }
        };

    setupIcon(iconWood, "assets/ui/icon_wood.png");
    setupIcon(iconFood, "assets/ui/icon_food.png");
    setupIcon(iconGold, "assets/ui/icon_gold.png");
    setupIcon(iconStone, "assets/ui/icon_stone.png");

    unsigned int fontSize = 18;

    auto setupText = [&](sf::Text& txt, sf::Color color) {
        txt.setFont(font);
        txt.setCharacterSize(fontSize);
        txt.setFillColor(color);
        txt.setOutlineThickness(1.5f);
        txt.setOutlineColor(sf::Color::Black);
        };

    setupText(woodText, sf::Color::White);
    setupText(foodText, sf::Color::White);
    setupText(goldText, sf::Color::White);
    setupText(stoneText, sf::Color::White);
    setupText(populationText, sf::Color::White);

    settingsButton.setSize(32, 32);

    sf::Texture& gearTex = AssetManager::getTexture("assets/ui/settingsButton.png");
    gearTex.setSmooth(true);
    settingsButton.setFillColor(sf::Color::Transparent);
    if (gearTex.getSize().x > 0) settingsButton.setTexture(gearTex);
}

void ResourceBar::setWidth(float width) {
    backgroundBar.setSize(sf::Vector2f(width, 40.f));

    if (hasTexture) {
        const sf::Texture* tex = barSprite.getTexture();
        if (tex) {
            float targetHeight = 40.0f;
            float scale = targetHeight / (float)tex->getSize().y;

            barSprite.setScale(scale, scale);
            barSprite.setTextureRect(sf::IntRect(0, 0, (int)(width / scale), tex->getSize().y));
        }

        float btnSize = 32.0f;
        float padding = 10.0f;
        settingsButton.setPosition(width - btnSize - padding, 4.0f);
    }

    float startX = 30.f;
    float spacing = width / 6.0f;
    float iconOffset = 40.0f;

    auto placeGroup = [&](sf::Sprite& icon, sf::Text& text, int index) {
        float groupX = startX + (spacing * index);
        icon.setPosition(groupX, 4);
        text.setPosition(groupX + iconOffset, 8);
        };
    placeGroup(iconWood, woodText, 0);
    placeGroup(iconFood, foodText, 1);
    placeGroup(iconGold, goldText, 2);
    placeGroup(iconStone, stoneText, 3);
    populationText.setPosition(startX + (spacing * 4), 8);
}

void ResourceBar::updateResources(int wood, int food, int gold, int stone, Player player) {
    woodText.setString(std::to_string(wood));
    foodText.setString(std::to_string(food));
    goldText.setString(std::to_string(gold));
    stoneText.setString(std::to_string(stone));
    populationText.setString(std::to_string(player.getUnitCount()) + "  /  " + std::to_string(player.getUnitLimit()));
}

void ResourceBar::draw(sf::RenderWindow& window) {
    if (hasTexture) window.draw(barSprite);
    else window.draw(backgroundBar);

    window.draw(iconWood); window.draw(woodText);
    window.draw(iconFood); window.draw(foodText);
    window.draw(iconGold); window.draw(goldText);
    window.draw(iconStone); window.draw(stoneText);
    window.draw(populationText);
    settingsButton.draw(window);
}

void ResourceBar::handleEvent(const sf::Event& event) {
    settingsButton.handleEvent(event);
}

void ResourceBar::setSettingsCallback(std::function<void()> cb) {
    settingsButton.setCallback(cb);
}