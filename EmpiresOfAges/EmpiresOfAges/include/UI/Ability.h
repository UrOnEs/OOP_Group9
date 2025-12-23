#pragma once
#include <string>
#include <functional>
#include <SFML/Graphics.hpp>

class Ability {
public:
    // Týklama fonksiyonu tipi (Callback)
    using Callback = std::function<void()>;

    Ability(int id, const std::string& name, const std::string& costText, const std::string& description, sf::Texture* icon)
        : m_id(id), m_name(name), m_costText(costText), m_description(description), m_icon(icon), m_onClick(nullptr)
    {
    }

    // --- GETTERS ---
    int getId() const { return m_id; }
    std::string getName() const { return m_name; }
    std::string getCostText() const { return m_costText; }
    std::string getDescription() const { return m_description; }
    sf::Texture* getIcon() const { return m_icon; }

    // --- ACTIONS ---
    // Týklandýðýnda ne olacaðýný ayarlar
    void setOnClick(Callback callback) { m_onClick = callback; }

    // Yeteneði çalýþtýrýr
    void execute() const {
        if (m_onClick) {
            m_onClick();
        }
    }

private:
    int m_id;
    std::string m_name;
    std::string m_costText;
    std::string m_description;
    sf::Texture* m_icon;

    Callback m_onClick;
};