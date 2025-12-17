#include "UIButton.h"

UIButton::UIButton()
    : m_isHovered(false), m_hasTexture(false), m_callback(nullptr) // Callback varsayýlan boþ
{
    m_shape.setFillColor(sf::Color(100, 100, 100));
    m_shape.setSize(sf::Vector2f(100, 50));
}

void UIButton::setPosition(float x, float y) {
    m_shape.setPosition(x, y);
    m_sprite.setPosition(x, y);

    // Yazýyý ortala
    sf::FloatRect textRect = m_text.getLocalBounds();
    m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    m_text.setPosition(x + m_shape.getSize().x / 2.0f, y + m_shape.getSize().y / 2.0f);
}

void UIButton::setSize(float width, float height) {
    m_shape.setSize(sf::Vector2f(width, height));
}

void UIButton::setText(const std::string& text, const sf::Font& font, unsigned int size) {
    m_text.setFont(font);
    m_text.setString(text);
    m_text.setCharacterSize(size);
    setPosition(m_shape.getPosition().x, m_shape.getPosition().y);
    m_text.setFillColor(sf::Color::Black);
}

void UIButton::setTexture(const sf::Texture& texture, float width, float height) {
    m_sprite.setTexture(texture);
    m_hasTexture = true;

    if (width > 0 && height > 0) {
        sf::Vector2u texSize = texture.getSize();
        m_sprite.setScale(width / (float)texSize.x, height / (float)texSize.y);
        m_shape.setSize(sf::Vector2f(width, height));
    }
}

void UIButton::setFillColor(const sf::Color& color) {
    if (m_hasTexture) m_sprite.setColor(color);
    else m_shape.setFillColor(color);
}

void UIButton::setCallback(std::function<void()> callback) {
    m_callback = callback;
}

// --- UIPanel ÝÇÝN KRÝTÝK OLAN FONKSÝYON ---
void UIButton::handleEvent(const sf::Event& event) {
    // 1. Mouse Hareketini Kontrol Et (Hover Efekti)
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos((float)event.mouseMove.x, (float)event.mouseMove.y);
        update(mousePos);
    }
    // 2. Týklamayý Kontrol Et (Callback Tetikle)
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);

        // Eðer mouse butonun üzerindeyse
        if (getGlobalBounds().contains(mousePos)) {
            // Ve bir fonksiyon atanmýþsa çalýþtýr
            if (m_callback) {
                m_callback();
            }
        }
    }
}

void UIButton::update(const sf::Vector2f& mousePos) {
    if (getGlobalBounds().contains(mousePos)) {
        m_isHovered = true;
        setFillColor(sf::Color(255, 255, 255, 255)); // Parlak
    }
    else {
        m_isHovered = false;
        setFillColor(sf::Color(200, 200, 200, 255)); // Normal
    }
}

bool UIButton::isMouseOver() const {
    return m_isHovered;
}

void UIButton::draw(sf::RenderWindow& window) {
    if (m_hasTexture) window.draw(m_sprite);
    else {
        window.draw(m_shape);
    }
    window.draw(m_text);
}

sf::FloatRect UIButton::getGlobalBounds() const {
    return m_hasTexture ? m_sprite.getGlobalBounds() : m_shape.getGlobalBounds();
}