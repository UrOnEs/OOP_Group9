#pragma once
#include "Building.h"
#include "Soldier.h"

class Barracks : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f;
    SoldierTypes currentProduction;

public:
    Barracks() {
        buildingType = BuildTypes::Barrack;
        health = GameRules::HP_Barracks;
        // Görselin origin noktasý (Görseli merkeze oturtmak için)
        this->sprite.setOrigin(64.f, 100.f);
    }

    // Class içine ekle:
    int getMaxHealth() const override { return (int)GameRules::HP_Barracks; }

    // Üretimi Baþlat
    void startTraining(SoldierTypes type, float duration) {
        if (!isProducing) {
            currentProduction = type;
            productionTimer = duration;
            productionDuration = duration;
            isProducing = true;
        }
    }

    // Süreyi Güncelle
    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    // Bitti mi?
    bool isReady() const { return isProducing && productionTimer <= 0; }

    // Üretimi Bitir
    SoldierTypes finishTraining() {
        isProducing = false;
        return currentProduction;
    }

    bool getIsProducing() const { return isProducing; }

    std::string getInfo() override { return "Barracks: Asker Uretir"; }

    // --------------------- RENDER  -----------------------
    void render(sf::RenderWindow& window) override {
        // Binayý çiz
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
            selectionCircle.setOrigin(radius, radius - 35);
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


        //  Üretim Barý
        if (isProducing) {
            float barWidth = 80.0f;
            float barHeight = 8.0f;

            sf::Vector2f pos = getPosition();
            float x = pos.x - barWidth / 2.0f;
            float y = pos.y - 70.0f; // Binanýn tepesi

            // Arka Plan
            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setPosition(x, y);
            backBar.setFillColor(sf::Color(50, 50, 50));
            backBar.setOutlineThickness(1);
            backBar.setOutlineColor(sf::Color::Black);

            // Ön Plan (Doluluk)
            float percent = 1.0f - (productionTimer / productionDuration);
            if (percent < 0) percent = 0; if (percent > 1) percent = 1;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * percent, barHeight));
            frontBar.setPosition(x, y);
            frontBar.setFillColor(sf::Color(255, 165, 0)); // Turuncu

            window.draw(backBar);
            window.draw(frontBar);
        }
    }
};