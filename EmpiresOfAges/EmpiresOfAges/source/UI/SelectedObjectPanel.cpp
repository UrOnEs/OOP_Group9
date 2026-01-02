#include "UI/SelectedObjectPanel.h"
#include <iostream>
#include "UI/Ability.h"
#include "UI/AssetManager.h"

SelectedObjectPanel::SelectedObjectPanel(float x, float y) {
    position = sf::Vector2f(x, y);

    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Handle font error
    }

    sf::Vector2f panelSize(500.0f, 140.0f);

    panelBackground.setSize(panelSize);
    panelBackground.setPosition(position);
    panelBackground.setFillColor(sf::Color(40, 30, 20, 240));
    panelBackground.setOutlineThickness(2.0f);
    panelBackground.setOutlineColor(sf::Color(200, 150, 50));

    sf::Texture& panelTex = AssetManager::getTexture("assets/ui/selectedObjectPanel_bg.png");
    if (panelTex.getSize().x > 0) {
        panelSprite.setTexture(panelTex);
        panelSprite.setPosition(position);
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

    nameText.setString(name);
    hpText.setString(std::to_string(health) + "/" + std::to_string(maxHealth));

    if (objectTexture) {
        selectedIcon.setTexture(*objectTexture, true);
        selectedIcon.setOrigin(0, 0);

        sf::Vector2u ts = objectTexture->getSize();
        if (ts.x > 0 && ts.y > 0) {
            selectedIcon.setScale(64.0f / ts.x, 64.0f / ts.y);
        }
    }

    float hpP = (maxHealth > 0) ? (float)health / maxHealth : 0.f;
    if (hpP > 1) hpP = 1; if (hpP < 0) hpP = 0;
    hpBarFront.setSize(sf::Vector2f(100 * hpP, 10));

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
            newBtn.setFillColor(sf::Color(139, 69, 19));
        }

        if (abilities[i].getIcon()) {
            newBtn.setTexture(*abilities[i].getIcon(), btnSize, btnSize);
        }
        else {
            newBtn.setSize(btnSize, btnSize);
            newBtn.setText("?", font, 20);
        }

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

    // Draw Production Queue
    if (!productionIcons.empty()) {
        float iconSize = 30.0f;
        float padding = 5.0f;
        float startX = position.x;
        float startY = position.y - iconSize - 10.0f;

        float totalWidth = (productionIcons.size() * (iconSize + padding)) + padding;
        float totalHeight = iconSize + 10.0f;

        sf::RectangleShape queuePanel;
        queuePanel.setSize(sf::Vector2f(totalWidth, totalHeight));
        queuePanel.setPosition(startX - padding, startY - padding);
        queuePanel.setFillColor(sf::Color(30, 30, 30, 220));
        queuePanel.setOutlineThickness(1.0f);
        queuePanel.setOutlineColor(sf::Color(180, 160, 100));
        window.draw(queuePanel);

        sf::Texture& slotBgTex = AssetManager::getTexture("assets/ui/production_bg.png");
        for (size_t i = 0; i < productionIcons.size(); ++i) {
            float itemX = startX + i * (iconSize + padding);

            sf::RectangleShape frameRect(sf::Vector2f(iconSize, iconSize));
            frameRect.setPosition(itemX, startY);

            if (slotBgTex.getSize().x > 0) {
                frameRect.setTexture(&slotBgTex);
                frameRect.setFillColor(sf::Color::White);
            }
            else {
                frameRect.setFillColor(sf::Color(60, 60, 60));
            }
            window.draw(frameRect);

            sf::RectangleShape iconRect(sf::Vector2f(iconSize, iconSize));
            iconRect.setPosition(startX + i * (iconSize + padding), startY);

            if (productionIcons[i]) {
                iconRect.setTexture(productionIcons[i]);
                iconRect.setFillColor(sf::Color::White);
            }
            else {
                iconRect.setFillColor(sf::Color(80, 80, 80));
            }
            iconRect.setOutlineThickness(1.0f);
            iconRect.setOutlineColor(sf::Color(50, 50, 50));
            window.draw(iconRect);

            // Progress bar for the first item
            if (i == 0) {
                sf::RectangleShape barBack(sf::Vector2f(iconSize, 4.0f));
                barBack.setPosition(startX, startY + iconSize + 2);
                barBack.setFillColor(sf::Color::Black);
                window.draw(barBack);

                sf::RectangleShape barFront(sf::Vector2f(iconSize * productionProgress, 4.0f));
                barFront.setPosition(startX, startY + iconSize + 2);
                barFront.setFillColor(sf::Color(0, 255, 0));
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

    float hpP = (maxHealth > 0) ? (float)health / maxHealth : 0.f;
    if (hpP > 1.0f) hpP = 1.0f;
    if (hpP < 0.0f) hpP = 0.0f;

    hpBarFront.setSize(sf::Vector2f(100.0f * hpP, 10.0f));
}