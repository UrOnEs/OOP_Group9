#pragma once
#include "Entity System/Entity.h"
#include "types.h"
#include "Game/GameRules.h" 
#include <vector>            
#include "UI/AssetManager.h" 

class Building : public Entity {
public:
    BuildTypes buildingType;
    bool isConstructed = true;

    Building() {
        health = GameRules::BuildingHealth;
    }

    virtual ~Building() = default;

    std::string stats() override {
        return getInfo();
    }

    virtual std::string getInfo() = 0;

    virtual void onConstructionComplete(class Player& owner) {}

    std::string getName() override {
        switch (buildingType) {
        case BuildTypes::Barrack:    return "Barracks";
        case BuildTypes::Farm:       return "Farm";
        case BuildTypes::WoodPlace:  return "Lumber Camp"; // Veya WoodPlace
        case BuildTypes::StoneMine:  return "Stone Mine";
        case BuildTypes::GoldMine:   return "Gold Mine";
        case BuildTypes::House:      return "House";
        case BuildTypes::TownCenter: return "Town Center";
        default:                     return "Building";
        }
    }

    // Ayrýca getIcon için basit bir yönlendirme yapabiliriz
    sf::Texture* getIcon() override {
        if (hasTexture) return (sf::Texture*)sprite.getTexture();
        return nullptr;
    }

    // Üretim sýrasýndaki birimlerin ikonlarýný döndürür.
    // Ýlk eleman = Þu an üretilen. Diðerleri = Sýrada bekleyenler.
    virtual std::vector<sf::Texture*> getProductionQueueIcons() {
        return {}; // Varsayýlan boþ
    }

    // Þu anki üretimin ilerleme yüzdesi (0.0 ile 1.0 arasý)
    virtual float getProductionProgress() {
        return 0.0f;
    }

    virtual void render(sf::RenderWindow& window) {
        if (!isConstructed) {
            sf::Sprite constructionSprite;
            // "assets/buildings/construction.png" resmini kullanýyoruz
            // Eðer bu resim yoksa varsayýlan bir þey döner, kod patlamaz.
            sf::Texture& tex = AssetManager::getTexture("assets/buildings/construction.png");
            constructionSprite.setTexture(tex);

            // --- SPRITE BOYUTLANDIRMA (FIX) ---
            // Asýl binanýn kapladýðý alaný alýyoruz
            sf::FloatRect buildingBounds = this->getBounds(); // Kendi boyutumuz
            sf::Vector2u texSize = tex.getSize();             // Ýnþaat resminin boyutu

            // Ölçek hesapla: Hedef Boyut / Resim Boyutu
            // Böylece resim ne kadar büyük olursa olsun binanýn içine sýðar.
            float scaleX = buildingBounds.width / (float)texSize.x;
            float scaleY = buildingBounds.height / (float)texSize.y;

            constructionSprite.setScale(scaleX, scaleY);

            // Pozisyonu ayarla (Merkezden merkeze)
            constructionSprite.setOrigin(texSize.x / 2.0f, texSize.y / 2.0f);
            constructionSprite.setPosition(getPosition());

            window.draw(constructionSprite);
        }
        else {
            // Normale dön
            if (hasTexture) sprite.setColor(sf::Color::White);
            else shape.setFillColor(sf::Color::White);
        }
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
            selectionCircle.setOrigin(radius, radius - 10);
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
        if (isConstructed) {
            if (hasTexture) {
                window.draw(sprite);
            }
            else {
                // Texture yoksa þekli çiz (Failsafe)
                window.draw(shape);
            }
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
};