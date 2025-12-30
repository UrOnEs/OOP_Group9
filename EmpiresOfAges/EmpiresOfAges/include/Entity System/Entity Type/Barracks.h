#pragma once
#include "Building.h"
#include "Soldier.h"
#include <deque>

class Barracks : public Building {
private:
    bool isProducing = false;
    float productionTimer = 0.0f;
    float productionDuration = 0.0f;
    SoldierTypes currentProduction;

    std::deque<SoldierTypes> productionQueue;

    sf::Texture* getUnitTexture(SoldierTypes type) {
        switch (type) {
        case SoldierTypes::Barbarian: return &AssetManager::getTexture("assets/units/barbarian.png");
        case SoldierTypes::Archer:    return &AssetManager::getTexture("assets/units/archer.png");
        case SoldierTypes::Wizard:    return &AssetManager::getTexture("assets/units/wizard.png");
        default: return nullptr;
        }
    }

public:
    Barracks() {
        buildingType = BuildTypes::Barrack;
        health = GameRules::HP_Barracks;
        // Görselin origin noktasý (Görseli merkeze oturtmak için)
        this->sprite.setOrigin(64.f, 100.f);

        addAbility(Ability(11, "Barbar Uret", "60 Yemek", "Temel Savasci", getUnitTexture(SoldierTypes::Barbarian)));
        addAbility(Ability(12, "Okçu Üret", "CostArcher", "Menzilli Asker", getUnitTexture(SoldierTypes::Archer)));
        addAbility(Ability(13, "Büyücü Üret", "CostWizard", "Uzun Menzilli Büyücü", getUnitTexture(SoldierTypes::Wizard)));
    }

    // Class içine ekle:
    int getMaxHealth() const override { return (int)GameRules::HP_Barracks; }

    //Production System için UI
    std::vector<sf::Texture*> getProductionQueueIcons() override {
        std::vector<sf::Texture*> icons;

        // 1. Aktif Üretim (Listenin Baþý)
        if (isProducing) {
            icons.push_back(getUnitTexture(currentProduction));
        }

        // 2. Bekleyen Kuyruk
        for (const auto& type : productionQueue) {
            icons.push_back(getUnitTexture(type));
        }
        return icons;
    }

    float getProductionProgress() override {
        if (!isProducing || productionDuration <= 0) return 0.0f;

        // 0'dan 1'e doðru artan deðer döndürür
        float progress = 1.0f - (productionTimer / productionDuration);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        return progress;
    }

    // Üretimi Baþlat
    void startTraining(SoldierTypes type, float duration) {
        if (isProducing) {
            // Zaten çalýþýyorsa sýraya at
            productionQueue.push_back(type);
        }
        else {
            // Boþsa hemen baþla
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
        SoldierTypes finishedUnit = currentProduction;

        // Kuyrukta asker var mý?
        if (!productionQueue.empty()) {
            // Sýradaki askeri al
            currentProduction = productionQueue.front();
            productionQueue.pop_front();

            // Süreyi yeniden baþlat (Sabit süre kullanýyoruz þimdilik)
            if (currentProduction == SoldierTypes::Barbarian) {
                productionDuration = 5.0f; // Barbarlar hýzlý
            } 
            else if(currentProduction == SoldierTypes::Archer) {
                productionDuration = 7.0f; 
            }
            else if (currentProduction == SoldierTypes::Wizard) {
                productionDuration = 10.0f;
            }
            else{
                productionDuration = 10.0f;
            }
            productionTimer = productionDuration;

            isProducing = true; // Üretim devam ediyor
        }
        else {
            // Kuyruk boþsa dur
            isProducing = false;
        }

        return finishedUnit; // Üretilen askeri sisteme ver
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
            Building::render(window);
        }
    }
};