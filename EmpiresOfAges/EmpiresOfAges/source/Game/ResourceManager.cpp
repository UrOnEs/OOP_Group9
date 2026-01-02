#include "Game/ResourceManager.h"

ResourceManager::ResourceManager() {
    Wood = 0;
    Gold = 0;
    Stone = 0;
    Food = 0;
}

ResourceManager::~ResourceManager() {
    textures.clear();
}

int ResourceManager::getAmount(ResourceType type) const {
    switch (type) {
    case ResourceType::Wood: return Wood;
    case ResourceType::Food: return Food;
    case ResourceType::Gold: return Gold;
    case ResourceType::Stone: return Stone;
    default: return 0;
    }
}

void ResourceManager::add(ResourceType type, int amount) {
    switch (type) {
    case ResourceType::Wood:  Wood += amount; break;
    case ResourceType::Food:  Food += amount; break;
    case ResourceType::Gold:  Gold += amount; break;
    case ResourceType::Stone: Stone += amount; break;
    }
}

bool ResourceManager::spend(ResourceType type, int amount) {
    switch (type) {
    case ResourceType::Wood:
        if (Wood >= amount) { Wood -= amount; return true; }
        break;
    case ResourceType::Food:
        if (Food >= amount) { Food -= amount; return true; }
        break;
    case ResourceType::Gold:
        if (Gold >= amount) { Gold -= amount; return true; }
        break;
    case ResourceType::Stone:
        if (Stone >= amount) { Stone -= amount; return true; }
        break;
    }
    return false;
}

void ResourceManager::loadTexture(const std::string& name, const std::string& fileName) {
    sf::Texture tex;
    if (tex.loadFromFile(fileName)) {
        textures[name] = tex;
    }
}

sf::Texture& ResourceManager::getTexture(const std::string& name) {
    return textures[name];
}