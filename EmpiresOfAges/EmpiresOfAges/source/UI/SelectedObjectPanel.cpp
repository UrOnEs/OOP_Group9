#include "UI/SelectedObjectPanel.h"
#include <iostream>
#include "UI/Ability.h"
#include "UI/AssetManager.h"

SelectedObjectPanel::SelectedObjectPanel(float x, float y) {
    position = sf::Vector2f(x, y);

    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Font hatasý yönetimi
    }

    sf::Vector2f panelSize(500.0f, 140.0f);

    // --- Panel Görsel Ayarlarý ---
    panelBackground.setSize(panelSize);
    panelBackground.setPosition(position);
    panelBackground.setFillColor(sf::Color(40, 30, 20, 240)); // Koyu Kahverengi
    panelBackground.setOutlineThickness(2.0f);
    panelBackground.setOutlineColor(sf::Color(200, 150, 50)); // Altýnýmsý çerçeve

    //Panel Texture yükleme
    sf::Texture& panelTex = AssetManager::getTexture("assets/ui/selectedObjectPanel_bg.png");
    if (panelTex.getSize().x > 0) { // Texture yüklendi mi kontrolü
        panelSprite.setTexture(panelTex);
        panelSprite.setPosition(position);

        // Paneli görsele göre ölçekle
        panelSprite.setScale(
            panelSize.x / (float)panelTex.getSize().x,
            panelSize.y / (float)panelTex.getSize().y
        );
        hasBackgroundTexture = true;
    }

    float infoStartX = position.x + 280;
    selectedIcon.setPosition(infoStartX, position.y + 20);

    nameText.setFont(font); nameText.setCharacterSize(18); nameText.setFillColor(sf::Color::White);
    nameText.setPosition(infoStartX + 70, position.y + 25);

    hpText.setFont(font); hpText.setCharacterSize(14); hpText.setFillColor(sf::Color::White);
    hpText.setPosition(infoStartX + 70, position.y + 80);

    hpBarBack.setSize(sf::Vector2f(100, 10)); hpBarBack.setFillColor(sf::Color(20, 20, 20));
    hpBarBack.setPosition(infoStartX + 70, position.y + 65);
    hpBarFront.setSize(sf::Vector2f(100, 10)); hpBarFront.setFillColor(sf::Color(0, 200, 0));
    hpBarFront.setPosition(infoStartX + 70, position.y + 65);

    tooltipBackground.setFillColor(sf::Color(0, 0, 0, 230));
    tooltipText.setFont(font); tooltipText.setCharacterSize(14); tooltipText.setFillColor(sf::Color::White);
    showTooltip = false;
    isVisible = false;
}

void SelectedObjectPanel::updateSelection(const std::string& name, int health, int maxHealth,
    sf::Texture* objectTexture,
    const std::vector<Ability>& abilities) {

    // --- Bilgileri Güncelle ---
    nameText.setString(name);
    hpText.setString(std::to_string(health) + "/" + std::to_string(maxHealth));

    if (objectTexture) {
        // DÜZELTME BURADA: ', true' parametresini ekliyoruz.
        // Bu, "Yeni texture'ýn boyutlarýna göre görünür alaný sýfýrla" demektir.
        selectedIcon.setTexture(*objectTexture, true);

        // Orijini sýfýrla (Her ihtimale karþý)
        selectedIcon.setOrigin(0, 0);

        // Ölçekleme mantýðý (Ayný kalýyor)
        sf::Vector2u ts = objectTexture->getSize();
        if (ts.x > 0 && ts.y > 0) {
            selectedIcon.setScale(64.0f / ts.x, 64.0f / ts.y);
        }
    }

    float hpP = (maxHealth > 0) ? (float)health / maxHealth : 0.f;
    if (hpP > 1) hpP = 1; if (hpP < 0) hpP = 0;
    hpBarFront.setSize(sf::Vector2f(100 * hpP, 10));

    // --- Butonlarý Oluþtur ---
    buttons.clear();
    currentAbilities = abilities;

    float startX = position.x + 25;
    float startY = position.y + 20;
    float btnSize = 50.0f;
    float padding = 10.0f;
    int columns = 4;

    sf::Texture& btnBgTex = AssetManager::getTexture("assets/ui/ability_bg.png");
    bool bgLoaded = (btnBgTex.getSize().x > 0);

    for (size_t i = 0; i < abilities.size(); ++i) {
        UIButton newBtn;
        
        newBtn.setSize(btnSize, btnSize);

        float bx = startX + (i % columns) * (btnSize + padding);
        float by = startY + (i / columns) * (btnSize + padding);
        newBtn.setPosition(bx, by);

        if (bgLoaded) {
            newBtn.setBackgroundTexture(btnBgTex);
        }
        else {
            // Görsel yoksa geçici olarak kahverengi yap ki yerini görelim
            newBtn.setFillColor(sf::Color(139, 69, 19));
        }

        if (abilities[i].getIcon()) {
            newBtn.setTexture(*abilities[i].getIcon(), btnSize, btnSize);
        }
        else {
            newBtn.setSize(btnSize, btnSize);
            newBtn.setText("?", font, 20);
        }

        // Callback'i kopyala
        Ability ab = abilities[i];
        newBtn.setCallback([ab]() {
            ab.execute();
            });

        buttons.push_back(newBtn);
    }
}

void SelectedObjectPanel::handleEvent(const sf::Event& event) {
    if (!isVisible) return;
    showTooltip = false;

    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].handleEvent(event);
        if (buttons[i].isMouseOver()) {
            setupTooltip(currentAbilities[i]);
            showTooltip = true;
        }
    }
}

void SelectedObjectPanel::setupTooltip(const Ability& info) {
    std::string txt = info.getName() + " (" + info.getCostText() + ")\n" + info.getDescription();
    tooltipText.setString(txt);

    sf::FloatRect b = tooltipText.getGlobalBounds();
    tooltipBackground.setSize(sf::Vector2f(b.width + 10, b.height + 10));

    // Tooltip panelin üstünde çýksýn
    float tx = position.x;
    float ty = position.y - b.height - 15;
    tooltipBackground.setPosition(tx, ty);
    tooltipText.setPosition(tx + 5, ty + 5);
}

void SelectedObjectPanel::updateQueue(const std::vector<sf::Texture*>& icons, float progress) {
    productionIcons = icons;
    productionProgress = progress;
}

void SelectedObjectPanel::draw(sf::RenderWindow& window) {
    if (!isVisible) return;
    if (hasBackgroundTexture) window.draw(panelSprite);
    else window.draw(panelBackground);
    window.draw(selectedIcon);
    window.draw(nameText);
    window.draw(hpText);
    window.draw(hpBarBack);
    window.draw(hpBarFront);

    for (auto& btn : buttons) btn.draw(window);

    // =============================================================
    //               ÜRETÝM KUYRUÐU ÇÝZÝMÝ (YENÝ)
    // =============================================================
    if (!productionIcons.empty()) {
        float iconSize = 30.0f;
        float padding = 5.0f;
        // Panelin sað tarafýnda, can barýnýn (y+80 civarý) altýnda boþluk var.
        float startX = position.x;
        float startY = position.y - iconSize - 10.0f;

        // --- 1. ADIM: ARKA PLAN KUTUSUNU OLUÞTUR ---
        // Kutunun geniþliði = (Ýkon sayýsý * Ýkon geniþliði) + kenar boþluklarý
        float totalWidth = (productionIcons.size() * (iconSize + padding)) + padding;
        float totalHeight = iconSize + 10.0f; // Yükseklik biraz daha fazla olsun (bar sýðsýn)

        sf::RectangleShape queuePanel;
        queuePanel.setSize(sf::Vector2f(totalWidth, totalHeight));
        queuePanel.setPosition(startX - padding, startY - padding); // Ýkonlarýn biraz solundan baþlat

        // Rengi ayarla (Koyu gri, hafif saydam)
        queuePanel.setFillColor(sf::Color(30, 30, 30, 220));

        // Çerçeve ekle (Altýnýmsý bir renk)
        queuePanel.setOutlineThickness(1.0f);
        queuePanel.setOutlineColor(sf::Color(180, 160, 100));

        // Önce kutuyu çiz (Ýkonlar bunun üstüne gelecek)
        window.draw(queuePanel);



        sf::Texture& slotBgTex = AssetManager::getTexture("assets/ui/production_bg.png");
        for (size_t i = 0; i < productionIcons.size(); ++i) {
            float itemX = startX + i * (iconSize + padding);

            // A) ÇERÇEVE (Slot Background)
            sf::RectangleShape frameRect(sf::Vector2f(iconSize, iconSize));
            frameRect.setPosition(itemX, startY);

            if (slotBgTex.getSize().x > 0) {
                frameRect.setTexture(&slotBgTex);
                frameRect.setFillColor(sf::Color::White);
            }
            else {
                frameRect.setFillColor(sf::Color(60, 60, 60)); // Doku yoksa gri kutu
            }
            window.draw(frameRect);

            // Ýkonun kendisi
            sf::RectangleShape iconRect(sf::Vector2f(iconSize, iconSize));
            iconRect.setPosition(startX + i * (iconSize + padding), startY);

            if (productionIcons[i]) {
                iconRect.setTexture(productionIcons[i]);
                iconRect.setFillColor(sf::Color::White);
            }
            else {
                iconRect.setFillColor(sf::Color(80, 80, 80)); // Resim yoksa gri kutu
            }

            // Ýkonun etrafýna ince bir çizgi
            iconRect.setOutlineThickness(1.0f);
            iconRect.setOutlineColor(sf::Color(50, 50, 50));

            window.draw(iconRect);

            // --- ÝLERLEME ÇUBUÐU (Sadece ilk sýradaki için) ---
            if (i == 0) {
                // Barýn arka planý (Siyah ince çizgi)
                sf::RectangleShape barBack(sf::Vector2f(iconSize, 4.0f));
                barBack.setPosition(startX, startY + iconSize + 2);
                barBack.setFillColor(sf::Color::Black);
                window.draw(barBack);

                // Barýn dolu kýsmý (Yeþil)
                sf::RectangleShape barFront(sf::Vector2f(iconSize * productionProgress, 4.0f));
                barFront.setPosition(startX, startY + iconSize + 2);
                barFront.setFillColor(sf::Color(0, 255, 0)); // Parlak Yeþil
                window.draw(barFront);
            }
        }
    }

    if (showTooltip) {
        window.draw(tooltipBackground);
        window.draw(tooltipText);
    }
}

void SelectedObjectPanel::setPosition(float x, float y) {
    float dx = x - position.x;
    float dy = y - position.y;
    position = sf::Vector2f(x, y);

    panelBackground.setPosition(position);
    if (hasBackgroundTexture) {
        panelSprite.setPosition(position);
    }


    selectedIcon.move(dx, dy);
    nameText.move(dx, dy);
    hpText.move(dx, dy);
    hpBarBack.move(dx, dy);
    hpBarFront.move(dx, dy);
}

bool SelectedObjectPanel::isMouseOver(float mouseX, float mouseY) const {
    if (!isVisible) return false;
    return panelBackground.getGlobalBounds().contains(mouseX, mouseY);
}

void SelectedObjectPanel::updateHealth(int health, int maxHealth) {
    
    hpText.setString(std::to_string(health) + "/" + std::to_string(maxHealth));

    // 2. Bar Boyutunu Güncelle
    float hpP = (maxHealth > 0) ? (float)health / maxHealth : 0.f;

    // Sýnýr korumasý (0 ile 1 arasýnda tut)
    if (hpP > 1.0f) hpP = 1.0f;
    if (hpP < 0.0f) hpP = 0.0f;

    hpBarFront.setSize(sf::Vector2f(100.0f * hpP, 10.0f));
}