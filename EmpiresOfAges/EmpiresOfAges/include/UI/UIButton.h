#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

/**
 * @brief A versatile UI button supporting text, background textures, and icons.
 */
class UIButton {
public:
    UIButton();

    void setPosition(float x, float y);
    void setSize(float width, float height);
    void setText(const std::string& text, const sf::Font& font, unsigned int size = 20, sf::Color color = sf::Color::Black);
    void setBackgroundTexture(const sf::Texture& texture);
    void setTexture(const sf::Texture& texture, float width = 0, float height = 0);
    void setFillColor(const sf::Color& color);

    /**
     * @brief Assigns a callback function triggered on left click.
     */
    void setCallback(std::function<void()> callback);

    void handleEvent(const sf::Event& event);
    void update(const sf::Vector2f& mousePos);
    bool isMouseOver() const;
    void draw(sf::RenderWindow& window);
    sf::FloatRect getGlobalBounds() const;

private:
    sf::RectangleShape m_shape;
    sf::Text m_text;

    sf::Sprite m_bgSprite;
    sf::Sprite m_sprite;

    bool m_isHovered;
    bool m_hasBgTexture;
    bool m_hasTexture;

    std::function<void()> m_callback;
};