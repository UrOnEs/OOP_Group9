#include "SelectedObjectPanel.h"

SelectedObjectPanel::SelectedObjectPanel() {
    font.loadFromFile("assets/fonts/arial.ttf");

    nameText.setFont(font);
    healthText.setFont(font);

    nameText.setPosition(10, 50);
    healthText.setPosition(10, 75);
}

void SelectedObjectPanel::updateSelection(const std::string& name, int health) {
    nameText.setString("Selected: " + name);
    healthText.setString("HP: " + std::to_string(health));
}

void SelectedObjectPanel::draw(sf::RenderWindow& window) {
    window.draw(nameText);
    window.draw(healthText);
}
