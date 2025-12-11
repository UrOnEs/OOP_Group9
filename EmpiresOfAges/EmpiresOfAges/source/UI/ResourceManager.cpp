#include "UI/ResourceManager.h"

std::map<std::string, sf::Texture> ResourceManager::textures;
std::map<std::string, sf::Font> ResourceManager::fonts;

sf::Texture& ResourceManager::getTexture(const std::string& filename) {
    if (textures.find(filename) == textures.end()) {
        textures[filename].loadFromFile(filename);
    }
    return textures[filename];
}

sf::Font& ResourceManager::getFont(const std::string& filename) {
    if (fonts.find(filename) == fonts.end()) {
        fonts[filename].loadFromFile(filename);
    }
    return fonts[filename];
}

