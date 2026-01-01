#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional> // std::function için gerekli

class UIButton {
public:
    UIButton();

    // Temel Ayarlar
    void setPosition(float x, float y);
    void setSize(float width, float height);
    void setText(const std::string& text, const sf::Font& font, unsigned int size = 20, sf::Color color = sf::Color::Black);

    void setBackgroundTexture(const sf::Texture& texture);
    // Resim (Sprite) Desteði
    void setTexture(const sf::Texture& texture, float width = 0, float height = 0);

    // Renk Ayarý
    void setFillColor(const sf::Color& color);

    // --- YENÝ EKLENEN: Callback ve Event Sistemi ---
    // Butona týklandýðýnda çalýþacak fonksiyonu ayarlar
    void setCallback(std::function<void()> callback);

    // UIPanel'in çalýþmasý için gerekli fonksiyonu geri getirdik
    void handleEvent(const sf::Event& event);

    // Manuel kontrol için yardýmcýlar (SelectedObjectPanel isterse bunlarý da kullanabilir)
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

    // Týklandýðýnda çalýþacak fonksiyonu saklar
    std::function<void()> m_callback;
};