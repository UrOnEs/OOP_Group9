#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

class UIButton {
public:
    UIButton(const sf::Vector2f& size, const sf::Vector2f& position);
    void setText(const std::string& text, sf::Font& font, unsigned int size = 20);
    void setCallback(const std::function<void()>& callback);

    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);

private:
    sf::RectangleShape shape;
    sf::Text label;
    std::function<void()> onClick;
};



