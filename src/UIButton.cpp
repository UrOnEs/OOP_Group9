#include "UIButton.h"

UIButton::UIButton(const sf::Vector2f& size, const sf::Vector2f& position) {
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(sf::Color(100, 100, 100));
}

void UIButton::setText(const std::string& text, sf::Font& font, unsigned int size) {
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(size);
    label.setFillColor(sf::Color::White);

    sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin(bounds.width / 2, bounds.height / 2);
    label.setPosition(
        shape.getPosition().x + shape.getSize().x / 2,
        shape.getPosition().y + shape.getSize().y / 2
    );
}

void UIButton::setCallback(const std::function<void()>& callback) {
    onClick = callback;
}

void UIButton::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {

        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
        if (shape.getGlobalBounds().contains(mousePos)) {
            if (onClick) onClick();
        }
    }
}

void UIButton::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(label);
}
