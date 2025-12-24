#include "UI/SelectedObjectPanel.h"
#include <iostream>
#include "UI/Ability.h" 

SelectedObjectPanel::SelectedObjectPanel(float x, float y) {
    position = sf::Vector2f(x, y);

    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Font hatasý yönetimi
    }

    // --- Panel Görsel Ayarlarý ---
    panelBackground.setSize(sf::Vector2f(500.0f, 140.0f));
    panelBackground.setPosition(position);
    panelBackground.setFillColor(sf::Color(40, 40, 40, 220));
    panelBackground.setOutlineThickness(2.0f);
    panelBackground.setOutlineColor(sf::Color(100, 100, 100));

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
        selectedIcon.setTexture(*objectTexture);
        sf::Vector2u ts = objectTexture->getSize();
        selectedIcon.setScale(64.0f / ts.x, 64.0f / ts.y);
    }

    float hpP = (maxHealth > 0) ? (float)health / maxHealth : 0.f;
    if (hpP > 1) hpP = 1; if (hpP < 0) hpP = 0;
    hpBarFront.setSize(sf::Vector2f(100 * hpP, 10));

    // --- Butonlarý Oluþtur ---
    buttons.clear();
    currentAbilities = abilities;

    float startX = position.x + 15;
    float startY = position.y + 15;
    float btnSize = 50.0f;
    float padding = 8.0f;
    int columns = 5;

    for (size_t i = 0; i < abilities.size(); ++i) {
        UIButton newBtn;
        float bx = startX + (i % columns) * (btnSize + padding);
        float by = startY + (i / columns) * (btnSize + padding);
        newBtn.setPosition(bx, by);

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

void SelectedObjectPanel::draw(sf::RenderWindow& window) {
    if (!isVisible) return;
    window.draw(panelBackground);
    window.draw(selectedIcon);
    window.draw(nameText);
    window.draw(hpText);
    window.draw(hpBarBack);
    window.draw(hpBarFront);

    for (auto& btn : buttons) btn.draw(window);

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