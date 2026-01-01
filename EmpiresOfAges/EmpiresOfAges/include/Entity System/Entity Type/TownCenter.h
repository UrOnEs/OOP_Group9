#pragma once
#include "Building.h"

class TownCenter : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f; // Ýlerleme çubuðu için gerekli toplam süre

    int queuedVillagers = 0;

public:
    TownCenter() {
        buildingType = BuildTypes::TownCenter;
        health = GameRules::HP_TownCenter;
        this->sprite.setOrigin(96.f, 150.f);
        addAbility(Ability(10, "Koylu Uret", "50 Food", "Kaynak toplar", &AssetManager::getTexture("assets/units/default.png")));
    }

    int getMaxHealth() const override { return (int)GameRules::HP_TownCenter; }

    //Production System UI
    std::vector<sf::Texture*> getProductionQueueIcons() override {
        std::vector<sf::Texture*> icons;
        // Köylü ikonunu al
        sf::Texture* villagerTex = &AssetManager::getTexture("assets/units/default.png");

        // 1. Aktif Üretim
        if (isProducing) {
            icons.push_back(villagerTex);
        }

        // 2. Bekleyenler (Sayý kadar ikon ekle)
        for (int i = 0; i < queuedVillagers; ++i) {
            icons.push_back(villagerTex);
        }

        return icons;
    }

    float getProductionProgress() override {
        if (!isProducing || productionDuration <= 0) return 0.0f;

        float progress = 1.0f - (productionTimer / productionDuration);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        return progress;
    }

    void startProduction(float duration) {
        
        if (!isConstructed) {
            std::cout << "[TOWN CENTER] Bina insaat halinde, asker uretemezsin!\n";
            return;
        }
        if (isProducing) {
            queuedVillagers++;
        }
        else {
            productionDuration = duration; // Gelen süreyi kullan
            productionTimer = productionDuration;
            isProducing = true;
        }
    }

    bool getIsProducing() const { return isProducing; }

    void updateTimer(float dt) {
        if (isProducing) productionTimer -= dt;
    }

    bool isReady() const { return isProducing && productionTimer <= 0; }
    
    void finishProduction() {
        if (queuedVillagers > 0) {
            // Sýrada bekleyen varsa, sayacý bir azalt ve süreyi sýfýrla
            queuedVillagers--;
            productionDuration = GameRules::Time_Build_Villager;
            productionTimer = productionDuration;
            isProducing = true; // Hala üretiyor
        }
        else {
            // Sýra bittiyse dur
            isProducing = false;
        }
    }

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
        
        Building::render(window);

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
            Building::render(window);
        }
    }
};