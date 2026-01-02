#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <list>
#include <string>

/**
 * @brief Global sound manager for handling sound effects and background music.
 */
class SoundManager {
public:
    /**
     * @brief Loads a sound file into memory.
     */
    static void loadSound(const std::string& name, const std::string& filename);

    /**
     * @brief Plays a sound effect (fire-and-forget).
     */
    static void playSound(const std::string& name);

    static void playMusic(const std::string& filename);
    static void setMusicVolume(float volume); // 0.0 to 100.0
    static float getMusicVolume();

    /**
     * @brief Updates internal lists, removing finished sounds. Call every frame.
     */
    static void update();

private:
    static std::map<std::string, sf::SoundBuffer> soundBuffers;
    static std::list<sf::Sound> activeSounds;
    static sf::Music backgroundMusic;
};