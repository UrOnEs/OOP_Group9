#include "UI/UIButton.h"
#include <algorithm>

UIButton::UIButton()
    : m_isHovered(false), m_hasBgTexture(false), m_hasTexture(false), m_callback(nullptr)
{
    m_shape.setFillColor(sf::Color(100, 100, 100));
    m_shape.setSize(sf::Vector2f(100, 50));
}

void UIButton::setPosition(float x, float y) {
    m_shape.setPosition(x, y);
    if (m_hasBgTexture) m_bgSprite.setPosition(x, y);

    if (m_hasTexture) {
        sf::FloatRect bgBounds = m_shape.getGlobalBounds();
        sf::FloatRect iconBounds = m_sprite.getGlobalBounds();
        float iconW = iconBounds.width;
        float iconH = iconBounds.height;

        float iconX = x + (bgBounds.width - iconW) / 2.0f;
        float iconY = y + (bgBounds.height - iconH) / 2.0f;
        m_sprite.setPosition(iconX, iconY);
    }

    sf::FloatRect textRect = m_text.getLocalBounds();
    m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    m_text.setPosition(x + m_shape.getSize().x / 2.0f, y + m_shape.getSize().y / 2.0f);
}

void UIButton::setSize(float width, float height) {
    m_shape.setSize(sf::Vector2f(width, height));

    if (m_hasBgTexture) {
        sf::Vector2u texSize = m_bgSprite.getTexture()->getSize();
        if (texSize.x > 0) {
            m_bgSprite.setScale(width / texSize.x, height / texSize.y);
        }
    }

    if (m_hasTexture) {
        sf::Vector2u texSize = m_sprite.getTexture()->getSize();
        if (texSize.x > 0) {
            float iconSize = std::min(width, height) * 0.7f;
            m_sprite.setScale(iconSize / texSize.x, iconSize / texSize.y);
        }
    }

    setPosition(m_shape.getPosition().x, m_shape.getPosition().y);
}

void UIButton::setBackgroundTexture(const sf::Texture& texture) {
    m_bgSprite.setTexture(texture);
    m_hasBgTexture = true;
    sf::Vector2f currentSize = m_shape.getSize();
    sf::Vector2u texSize = texture.getSize();

    if (texSize.x > 0 && currentSize.x > 0) {
        m_bgSprite.setScale(currentSize.x / texSize.x, currentSize.y / texSize.y);
    }
}

void UIButton::setText(const std::string& text, const sf::Font& font, unsigned int size, sf::Color color) {
    m_text.setFont(font);
    m_text.setString(text);
    m_text.setCharacterSize(size);
    m_text.setFillColor(color);
    setPosition(m_shape.getPosition().x, m_shape.getPosition().y);
}

void UIButton::setTexture(const sf::Texture& texture, float width, float height) {
    m_sprite.setTexture(texture);
    m_hasTexture = true;

    sf::Vector2f btnSize = m_shape.getSize();
    sf::Vector2u texSize = texture.getSize();

    if (texSize.x > 0) {
        m_sprite.setScale(btnSize.x / texSize.x, btnSize.y / texSize.y);
    }
    setPosition(m_shape.getPosition().x, m_shape.getPosition().y);
}

void UIButton::setFillColor(const sf::Color& color) {
    if (m_hasTexture) m_sprite.setColor(color);
    else m_shape.setFillColor(color);
}

void UIButton::setCallback(std::function<void()> callback) {
    m_callback = callback;
}

void UIButton::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos((float)event.mouseMove.x, (float)event.mouseMove.y);
        update(mousePos);
    }
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);

        if (getGlobalBounds().contains(mousePos)) {
            if (m_callback) {
                m_callback();
            }
        }
    }
}

void UIButton::update(const sf::Vector2f& mousePos) {
    if (getGlobalBounds().contains(mousePos)) {
        m_isHovered = true;
        setFillColor(sf::Color(255, 255, 255, 255));
    }
    else {
        m_isHovered = false;
        setFillColor(sf::Color(200, 200, 200, 255));
    }
}

bool UIButton::isMouseOver() const {
    return m_isHovered;
}

void UIButton::draw(sf::RenderWindow& window) {
    if (m_hasBgTexture) window.draw(m_bgSprite);
    else window.draw(m_shape);

    if (m_hasTexture) window.draw(m_sprite);
    window.draw(m_text);
}

sf::FloatRect UIButton::getGlobalBounds() const {
    return m_hasTexture ? m_sprite.getGlobalBounds() : m_shape.getGlobalBounds();
}