#include "UI/AssetManager.h"

std::map<std::string, sf::Texture> AssetManager::textures;
std::map<std::string, sf::Font> AssetManager::fonts;

sf::Texture& AssetManager::getTexture(const std::string& filename) {
    if (textures.find(filename) == textures.end()) {
        textures[filename].loadFromFile(filename);
    }
    return textures[filename];
}

sf::Font& AssetManager::getFont(const std::string& filename) {
    if (fonts.find(filename) == fonts.end()) {
        fonts[filename].loadFromFile(filename);
    }
    return fonts[filename];
}

