#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Game/TeamColors.h"

class Entity {
protected:
    // 1. ASIL OYUNCU: Sprite (Resim varsa bu çizilecek)
    sf::Sprite sprite;
    bool hasTexture = false;

    // 2. YEDEK OYUNCU: Yuvarlak (Resim yoksa bu çizilecek)
    sf::CircleShape shape;

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

    // "getModel" hatasý için (Eski kodlar shape'e model diyordu)
    sf::CircleShape& getShape() { return shape; }

    // "getRange" hatasý için
    float getRange() const { return range; }

    // --- ÇÝZÝM (Render) ---
    // Bunu .h içinde tanýmladýðýmýz için .cpp'de tekrar yazma!
    virtual void render(sf::RenderWindow& window) {
        if (!isAlive) return;

        // Seçim Halkasý
        if (isSelected) {
            shape.setOutlineThickness(2.0f);
            shape.setOutlineColor(sf::Color::Green);
        }
        else {
            shape.setOutlineThickness(0.0f);
        }

        if (hasTexture) {
            window.draw(sprite);
            if (isSelected) window.draw(shape); // Seçiliyse halkayý da çiz
        }
        else {
            window.draw(shape);
        }
    }

    // --- DURUM YÖNETÝMÝ (Eksik olanlar buradaydý) ---

    // "getIsAlive" hatasý için
    bool getIsAlive() const { return isAlive; }

    // "setSelected" hatasý için
    void setSelected(bool status) { isSelected = status; }

    // "takeDamage" hatasý için
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