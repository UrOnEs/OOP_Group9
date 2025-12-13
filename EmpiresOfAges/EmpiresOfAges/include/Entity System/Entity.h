#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <iostream>
#include <memory>
#include <SFML/System/Vector2.hpp>
#include "Game/TeamColors.h"
#include <SFML/Graphics.hpp>

class Entity {
protected:

    //ait olduðu takým renkleri (bunun sayesinde kim kime saldýrabilir falan görücez)
    TeamColors Color;

    // oyun içi görüntüsü
    sf::Sprite model;

    sf::RectangleShape shape;

    // arayüzdeki sembol
    sf::Sprite icon;

    sf::Vector2f position; // Custom Vector2 yerine sf::Vector2f

public:
    int entityID;
    float health;
    float damage;
    float range;
    bool isAlive;
    bool isSelected;

    void move(const sf::Vector2f& offset) {
        setPosition(getPosition() + offset);
    }

    virtual std::string stats() = 0;

    virtual ~Entity() = default;

    // --- GETTER & SETTER (Sistemlerin veriyi okumasý için þart) ---
    sf::Vector2f getPosition() const { return position; }


    void setPosition(const sf::Vector2f& newPos) {
        position = newPos;      // Mantýksal konumu güncelle
        model.setPosition(newPos); // Görsel konumu güncelle (Ekranda hareket etmesi için þart)
    }


    float getRange() const { return range; } // Saldýrý sistemi için lazým olacak

    // Saðlýk sistemi için
    void takeDamage(float amount) {
        health -= amount;
        if (health <= 0) isAlive = false;
    }
    bool getIsAlive() const { return isAlive; }


    // Modelin kendisine ulaþmak için (Çizim yaparken lazým olacak)
    sf::Sprite& getModel() { return model; }

    // Ýkonun kendisine ulaþmak için (UI)
    sf::Sprite& getIcon() { return icon; }

    // Dýþarýdan texture atamak için 
    void setTexture(const sf::Texture& texture) {
        model.setTexture(texture);
        // Resim yüklendiðinde merkezini ortalamak iyi bir taktiktir:
        model.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
    }


    void setIconTexture(const sf::Texture& texture) {
        icon.setTexture(texture);
    }

    virtual void render(sf::RenderWindow& window);

    sf::FloatRect getBounds() {
        return model.getGlobalBounds(); // Sprite'ýn kapladýðý alan
    }

    //Seçildiðini görselleþtirmek için:
    void setSelected(bool status) {
        isSelected = status;
        if (isSelected) model.setColor(sf::Color::Green); // Seçilince yeþil olsun
        else model.setColor(sf::Color::White); // Normale dönsün
    }
};

#endif