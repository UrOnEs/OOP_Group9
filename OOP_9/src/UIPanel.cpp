#include "UIPanel.h"

UIPanel::UIPanel(const sf::Vector2f& size, const sf::Vector2f& position) {
    background.setSize(size);
    background.setPosition(position);
    background.setFillColor(sf::Color(50, 50, 50, 200));
}

void UIPanel::addButton(const UIButton& button) {
    buttons.push_back(button);
}

void UIPanel::handleEvent(const sf::Event& event) {
    for (auto& b : buttons)
        b.handleEvent(event);
}

void UIPanel::draw(sf::RenderWindow& window) {
    window.draw(background);
    for (auto& b : buttons)
        b.draw(window);
}

