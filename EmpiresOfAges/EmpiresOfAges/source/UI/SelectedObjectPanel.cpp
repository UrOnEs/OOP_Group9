#include "UI/SelectedObjectPanel.h"
#include <iostream>

SelectedObjectPanel::SelectedObjectPanel(float x, float y) {
    position = sf::Vector2f(x, y);

    // Font y?kleme
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Font yuklenemedi: assets/fonts/arial.ttf" << std::endl;
    }

    // --- 1. ARKA PLAN AYARLARI ---
    panelBackground.setSize(sf::Vector2f(450.0f, 120.0f));
    panelBackground.setPosition(position);
    panelBackground.setFillColor(sf::Color(40, 40, 40, 220)); // Koyu gri, hafif saydam
    panelBackground.setOutlineThickness(2.0f);
    panelBackground.setOutlineColor(sf::Color(100, 100, 100));

    // --- 2. SA? TARAFTAK? B?LG? PANEL? (INFO) ---
    float infoStartX = position.x + 280;

    // Se?ili Obje ?konu
    selectedIcon.setPosition(infoStartX, position.y + 20);

    // ?sim
    nameText.setFont(font);
    nameText.setCharacterSize(18);
    nameText.setFillColor(sf::Color::White);
    nameText.setPosition(infoStartX + 70, position.y + 25);

    // Can De?eri Metni
    hpText.setFont(font);
    hpText.setCharacterSize(14);
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition(infoStartX + 70, position.y + 80);

    // Can Bar? Arka Plan?
    hpBarBack.setSize(sf::Vector2f(100, 10));
    hpBarBack.setFillColor(sf::Color(20, 20, 20));
    hpBarBack.setPosition(infoStartX + 70, position.y + 65);

    // Can Bar? ?n Y?z? (Ye?il)
    hpBarFront.setSize(sf::Vector2f(100, 10));
    hpBarFront.setFillColor(sf::Color(0, 200, 0));
    hpBarFront.setPosition(infoStartX + 70, position.y + 65);

    // --- 3. TOOLTIP (?PUCU KUTUSU) ---
    tooltipBackground.setFillColor(sf::Color(0, 0, 0, 230));
    tooltipText.setFont(font);
    tooltipText.setCharacterSize(14);
    tooltipText.setFillColor(sf::Color::White);
    showTooltip = false;
}

void SelectedObjectPanel::updateSelection(const std::string& name, int health, int maxHealth,
    sf::Texture* objectTexture,
    const std::vector<AbilityInfo>& abilities) {
    // --- A. B?LG?LER? G?NCELLE ---
    nameText.setString(name);

    // Ana ?konu Ayarla ve ?l?ekle (64x64'e s??d?r)
    if (objectTexture) {
        selectedIcon.setTexture(*objectTexture);
        sf::Vector2u texSize = objectTexture->getSize();
        selectedIcon.setScale(64.0f / texSize.x, 64.0f / texSize.y);
    }

    // Can Bar?n? G?ncelle
    hpText.setString(std::to_string(health) + "/" + std::to_string(maxHealth));
    float hpPercent = (maxHealth > 0) ? (float)health / (float)maxHealth : 0.0f;
    hpBarFront.setSize(sf::Vector2f(100 * hpPercent, 10));


    // --- B. BUTONLARI OLU?TUR (UIButton Kullanarak) ---
    buttons.clear();
    currentAbilities = abilities; // Tooltip i?in veriyi sakla

    float startX = position.x + 15;
    float startY = position.y + 15;
    float btnSize = 50.0f;
    float padding = 8.0f;
    int columns = 5;

    for (size_t i = 0; i < abilities.size(); ++i) {
        UIButton newBtn;

        // Grid Pozisyonu Hesapla
        float btnX = startX + (i % columns) * (btnSize + padding);
        float btnY = startY + (i / columns) * (btnSize + padding);

        newBtn.setPosition(btnX, btnY);

        // Texture Varsa Y?kle, Yoksa "?" Koy
        if (abilities[i].iconTexture) {
            newBtn.setTexture(*abilities[i].iconTexture, btnSize, btnSize);
        }
        else {
            newBtn.setSize(btnSize, btnSize);
            newBtn.setText("?", font, 20);
        }

        // --- CALLBACK (T?klama G?revi) ATA ---
        int actionID = abilities[i].id;
        newBtn.setCallback([actionID]() {
            // Buras? butona t?kland???nda ?al??acak kod blo?u
            std::cout << "[PANEL] Action Button Clicked! ID: " << actionID << std::endl;

            // ?leride buraya NetworkManager->sendCommand(...) gelecek.
            });

        buttons.push_back(newBtn);
    }
}

void SelectedObjectPanel::handleEvent(const sf::Event& event) {
    showTooltip = false;

    // T?m butonlar?n olaylar?n? (hover/click) tetikle
    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].handleEvent(event);

        // E?er mouse ?zerindeyse Tooltip g?ster (MouseMoved event'inde bak?l?r)
        if (buttons[i].isMouseOver()) {
            setupTooltip(currentAbilities[i]);
            showTooltip = true;
        }
    }
}

void SelectedObjectPanel::setupTooltip(const AbilityInfo& info) {
    // Tooltip metnini haz?rla
    std::string fullText = info.name + " (" + info.costText + ")\n\n" + info.description;
    tooltipText.setString(fullText);

    // Arka plan? metne g?re boyutland?r
    sf::FloatRect textBounds = tooltipText.getGlobalBounds();
    tooltipBackground.setSize(sf::Vector2f(textBounds.width + 20, textBounds.height + 20));

    // Tooltip'i panelin hemen ?st?ne yerle?tir
    float tooltipX = position.x;
    float tooltipY = position.y - tooltipBackground.getSize().y - 5;

    tooltipBackground.setPosition(tooltipX, tooltipY);
    tooltipText.setPosition(tooltipX + 10, tooltipY + 10);
}

void SelectedObjectPanel::draw(sf::RenderWindow& window) {
    // 1. Arka Plan
    window.draw(panelBackground);

    // 2. Bilgi K?sm?
    window.draw(nameText);
    window.draw(hpText);
    window.draw(selectedIcon);
    window.draw(hpBarBack);
    window.draw(hpBarFront);

    // 3. Butonlar (UIButton s?n?f?n?n kendi draw fonksiyonu)
    for (auto& btn : buttons) {
        btn.draw(window);
    }

    // 4. Tooltip (En ?stte)
    if (showTooltip) {
        window.draw(tooltipBackground);
        window.draw(tooltipText);
    }
}