#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Game/TeamColors.h"
#include "UI/Ability.h"
#include "UI/AssetManager.h"

class Entity {
protected:
    // 1. ASIL OYUNCU: Sprite (Resim varsa bu çizilecek)
    sf::Sprite sprite;
    bool hasTexture = false;
    std::vector<Ability> m_abilities;

    // 2. YEDEK OYUNCU: Yuvarlak (Resim yoksa bu çizilecek)
    sf::CircleShape shape;

    // TAKIM RENGÝ
    TeamColors m_team = TeamColors::Blue;

public:
    int entityID;
    float health;
    float damage;
    float range;
    bool isAlive;
    bool isSelected;

    Entity() {
        isAlive = true;
        isSelected = false;
        hasTexture = false;

        // Varsayýlan Yuvarlak Ayarlarý
        shape.setRadius(15.f);
        shape.setOrigin(15.f, 15.f);
        shape.setPointCount(30);
        shape.setFillColor(sf::Color::White); // Varsayýlan renk
    }

    virtual int getMaxHealth() const = 0;

    virtual ~Entity() = default;

    // --- HAREKET ---
    void move(const sf::Vector2f& offset) {
        if (hasTexture) sprite.move(offset);
        shape.move(offset);
    }

    virtual void setPosition(const sf::Vector2f& newPos) {
        if (hasTexture) sprite.setPosition(newPos);
        shape.setPosition(newPos);
    }

    void setScale(float x, float y) {
        if (hasTexture) sprite.setScale(x, y);
        shape.setScale(x, y);
    }

    // UI Ýçin Gerekli Fonksiyonlar (Hata veren kýsýmlar)
    virtual std::string getName() { return "Bilinmeyen"; }
    virtual sf::Texture* getIcon() { return nullptr; }
    void addAbility(const Ability& ability) {
        m_abilities.push_back(ability);
    }
    std::vector<Ability> getAbilities() const {
        return m_abilities;
    }

    virtual sf::Vector2f getPosition() const {
        return hasTexture ? sprite.getPosition() : shape.getPosition();
    }

    // --- GÖRÜNÜM ---
    void setTexture(const sf::Texture& texture) {
        sprite.setTexture(texture);
        sf::Vector2u size = texture.getSize();
        sprite.setOrigin(size.x / 2.f, size.y / 2.f);
        hasTexture = true;
    }

    void setTeam(TeamColors newTeam) {
        m_team = newTeam;

        // Rengi þekle de yansýt (Sprite yoksa yuvarlaðýn rengi deðiþsin)
        sf::Color c = sf::Color::White;
        if (m_team == TeamColors::Red) c = sf::Color::Red;
        else if (m_team == TeamColors::Blue) c = sf::Color(0, 100, 255); // Güzel bir mavi
        else if (m_team == TeamColors::Green) c = sf::Color::Green;
        else if (m_team == TeamColors::Purple) c = sf::Color::Magenta;

        shape.setFillColor(c);
        // Ýstersen sprite'ý da hafif boyayabilirsin:
        if (hasTexture) sprite.setColor(c); 
    }

    TeamColors getTeam() const { return m_team; }

    // "getModel" hatasý için (Eski kodlar shape'e model diyordu)
    sf::CircleShape& getShape() { return shape; }

    // "getRange" hatasý için
    float getRange() const { return range; }

    // --- ÇÝZÝM (Render) ---
    virtual void render(sf::RenderWindow& window) {
        // 1. SEÇÝM HALKASINI ÇÝZ (Seçiliyse)
        if (isSelected) {
            float radius = 32.0f; // Varsayýlan bir boyut (Askerler için)

            // Eðer bir texture (görsel) varsa, onun boyutuna göre ayarla
            if (hasTexture) {
                // Sprite'ýn ekranda kapladýðý alaný al (Ölçeklenmiþ hali)
                sf::FloatRect bounds = sprite.getGlobalBounds();
                // Yarýçapý geniþliðin yarýsý yap. Biraz taþmasý için 1.1 ile çarpabiliriz.
                radius = (bounds.width / 2.0f) * 1.1f;
            }

            sf::CircleShape selectionCircle(radius);
            selectionCircle.setPointCount(40); // Daha pürüzsüz bir daire

            // Merkezi ayarla (Tam ayak bastýðý nokta)
            selectionCircle.setOrigin(radius, radius-10);
            selectionCircle.setPosition(getPosition());

            // --- ÝZOMETRÝK GÖRÜNÜM ---
            // Daireyi dikey olarak bastýrarak elips yapýyoruz.
            // Yere yatýrýlmýþ gibi görünmesini saðlar.
            selectionCircle.setScale(1.0f, 0.6f);

            // Renk ve þeffaflýk ayarlarý
            selectionCircle.setFillColor(sf::Color(0, 255, 0, 50)); // Ýçi yarý saydam yeþil
            selectionCircle.setOutlineColor(sf::Color::Green);      // Kenarý parlak yeþil
            selectionCircle.setOutlineThickness(3.0f);            // Çizgi kalýnlýðý (Daha belirgin)

            window.draw(selectionCircle);
        }

        // 2. ENTITY'NÝN KENDÝSÝNÝ ÇÝZ
        if (hasTexture) {
            window.draw(sprite);
        }
        else {
            // Texture yoksa þekli çiz (Failsafe)
            window.draw(shape);
        }

        if (isAlive && (isSelected || health < getMaxHealth())) {
            float barWidth = 40.0f;
            float barHeight = 5.0f;

            sf::Vector2f pos = getPosition();
            // Sprite'ýn tepesine koy
            if (hasTexture) pos.y -= sprite.getGlobalBounds().height / 2.0f + 10.0f;
            else pos.y -= 25.0f;

            // Arka Plan (Kýrmýzý)
            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setOrigin(barWidth / 2, barHeight / 2);
            backBar.setPosition(pos);
            backBar.setFillColor(sf::Color(100, 0, 0));

            // Ön Plan (Yeþil/Sarý)
            float hpPercent = (float)health / getMaxHealth();
            if (hpPercent < 0) hpPercent = 0;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * hpPercent, barHeight));
            frontBar.setOrigin(barWidth / 2, barHeight / 2);
            frontBar.setPosition(pos.x - (barWidth * (1 - hpPercent)) / 2.0f, pos.y); // Soldan hizala

            if (hpPercent > 0.5f) frontBar.setFillColor(sf::Color::Green);
            else if (hpPercent > 0.25f) frontBar.setFillColor(sf::Color::Yellow);
            else frontBar.setFillColor(sf::Color::Red);

            window.draw(backBar);
            window.draw(frontBar);
        }
    }

    virtual void renderEffects(sf::RenderWindow& window) {}

    // --- DURUM YÖNETÝMÝ ---

    bool getIsAlive() const { return isAlive; }

    void setSelected(bool status) { isSelected = status; }

    void takeDamage(float amount) {
        health -= amount;
        if (health <= 0) isAlive = false;
    }

    // Týklama Alaný
    sf::FloatRect getBounds() const {
        return hasTexture ? sprite.getGlobalBounds() : shape.getGlobalBounds();
    }

    virtual std::string stats() = 0;
};

#endif