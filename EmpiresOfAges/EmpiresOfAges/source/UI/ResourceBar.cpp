#include "ResourceBar.h"

ResourceBar::ResourceBar() {
    font.loadFromFile("assets/fonts/arial.ttf");

    woodText.setFont(font);
    foodText.setFont(font);
    goldText.setFont(font);
    stoneText.setFont(font);

    woodText.setPosition(10, 5);
    foodText.setPosition(120, 5);
    goldText.setPosition(230, 5);
    stoneText.setPosition(340, 5);
}

void ResourceBar::updateResources(int wood, int food, int gold, int stone) {
    woodText.setString("Wood: " + std::to_string(wood));
    foodText.setString("Food: " + std::to_string(food));
    goldText.setString("Gold: " + std::to_string(gold));
    stoneText.setString("Stone: " + std::to_string(stone));
}

void ResourceBar::draw(sf::RenderWindow& window) {
    window.draw(woodText);
    window.draw(foodText);
    window.draw(goldText);
    window.draw(stoneText);
}

