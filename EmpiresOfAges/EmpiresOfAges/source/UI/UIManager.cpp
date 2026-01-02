#include "UI/UIManager.h"

void UIManager::addPanel(const UIPanel& panel) {
    panels.push_back(panel);
}

void UIManager::handleEvent(const sf::Event& event) {
    for (auto& p : panels)
        p.handleEvent(event);
}

void UIManager::draw(sf::RenderWindow& window) {
    for (auto& p : panels)
        p.draw(window);
}