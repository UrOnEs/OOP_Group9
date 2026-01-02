#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

/**
 * @brief Static resource manager to load and store textures and fonts.
 * Prevents loading the same asset multiple times.
 */
class AssetManager {
public:
    /**
     * @brief Retrieves a texture by filename. Loads it if not already cached.
     */
    static sf::Texture& getTexture(const std::string& filename);

    /**
     * @brief Retrieves a font by filename. Loads it if not already cached.
     */
    static sf::Font& getFont(const std::string& filename);

private:
    static std::map<std::string, sf::Texture> textures;
    static std::map<std::string, sf::Font> fonts;
};