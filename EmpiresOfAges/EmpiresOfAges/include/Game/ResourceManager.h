#pragma once
#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include "Game/GameRules.h"

enum class ResourceType {
    Wood,
    Food,
    Gold,
    Stone
};

/**
 * @brief Manages resource inventory for a player.
 */
class ResourceManager {
private:
    int Wood = 0;
    int Gold = 0;
    int Stone = 0;
    int Food = 0;

    std::map<std::string, sf::Texture> textures;

public:
    ResourceManager();
    ~ResourceManager();

    int getAmount(ResourceType type) const;
    void add(ResourceType type, int amount);
    bool spend(ResourceType type, int amount);

    bool canAfford(const GameRules::Cost& cost) const {
        if (Wood < cost.wood) return false;
        if (Food < cost.food) return false;
        if (Gold < cost.gold) return false;
        if (Stone < cost.stone) return false;
        return true;
    }

    void pay(const GameRules::Cost& cost) {
        Wood -= cost.wood;
        Food -= cost.food;
        Gold -= cost.gold;
        Stone -= cost.stone;
    }

    void loadTexture(const std::string& name, const std::string& fileName);
    sf::Texture& getTexture(const std::string& name);
};