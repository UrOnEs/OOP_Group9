#pragma once
#include "Building.h"

class TownCenter : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f; // Ýlerleme çubuðu için gerekli toplam süre

public:
    TownCenter() {
        buildingType = BuildTypes::TownCenter;
        health = GameRules::HP_TownCenter;
        this->sprite.setOrigin(96.f, 150.f);
    }

    int getMaxHealth() const override { return (int)GameRules::HP_TownCenter; }

    void startProduction() {
        if (!isProducing) {
            productionDuration = GameRules::Time_Build_Villager; // Süreyi kaydet
            productionTimer = productionDuration;
            isProducing = true;
        }
    }

    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    bool isReady() const { return isProducing && productionTimer <= 0; }
    void finishProduction() { isProducing = false; }

    std::string getInfo() override { return "TownCenter: Ana Merkez"; }

    // --- YENÝ: RENDER FONKSÝYONU (Bar Çizimi) ---
    void render(sf::RenderWindow& window) override {
        // 1. Binanýn kendisini çiz
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
            selectionCircle.setOrigin(radius, radius - 60);
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

        // 2. Üretim Barýný Çiz
        if (isProducing) {
            float barWidth = 100.0f; // Bina büyük, bar da büyük olsun
            float barHeight = 10.0f;

            sf::Vector2f pos = getPosition();
            float x = pos.x - barWidth / 2.0f;
            float y = pos.y - 120.0f; // Binanýn epey tepesine (3x3 olduðu için)

            // Arka Plan (Siyah)
            sf::RectangleShape backBar(sf::Vector2f(barWidth, barHeight));
            backBar.setPosition(x, y);
            backBar.setFillColor(sf::Color(50, 50, 50));
            backBar.setOutlineThickness(1);
            backBar.setOutlineColor(sf::Color::Black);

            // Ön Plan (Mavi - Köylü üretimi için farklý renk)
            float percent = 1.0f - (productionTimer / productionDuration);
            if (percent < 0) percent = 0; if (percent > 1) percent = 1;

            sf::RectangleShape frontBar(sf::Vector2f(barWidth * percent, barHeight));
            frontBar.setPosition(x, y);
            frontBar.setFillColor(sf::Color(0, 191, 255)); // Deep Sky Blue

            window.draw(backBar);
            window.draw(frontBar);
        }
    }
};