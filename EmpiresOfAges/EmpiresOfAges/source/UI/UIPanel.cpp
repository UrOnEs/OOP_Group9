#include "UI/UIPanel.h"

UIPanel::UIPanel(const sf::Vector2f& size, const sf::Vector2f& position) {
    background.setSize(size);
    background.setPosition(position);
    background.setFillColor(sf::Color(50, 50, 50, 200));
    hasTexture = false;
}

void UIPanel::setTexture(const sf::Texture& texture) {
    backgroundSprite.setTexture(texture);
    backgroundSprite.setPosition(background.getPosition());

    sf::Vector2u texSize = texture.getSize();
    sf::Vector2f panelSize = background.getSize();

    if (texSize.x > 0 && texSize.y > 0) {
        backgroundSprite.setScale(
            panelSize.x / (float)texSize.x,
            panelSize.y / (float)texSize.y
        );
    }
    hasTexture = true;
}

void UIPanel::addButton(const UIButton& button) {
    buttons.push_back(button);
}

void UIPanel::handleEvent(const sf::Event& event) {
    for (auto& b : buttons)
        b.handleEvent(event);
}

void UIPanel::draw(sf::RenderWindow& window) {
    if (hasTexture) {
        window.draw(backgroundSprite);
    }
    else {
        window.draw(background);
    }

    for (auto& b : buttons)
        b.draw(window);
}