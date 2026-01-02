#include "Systems/SoundManager.h"
#include <iostream>

std::map<std::string, sf::SoundBuffer> SoundManager::soundBuffers;
std::list<sf::Sound> SoundManager::activeSounds;
sf::Music SoundManager::backgroundMusic;

void SoundManager::loadSound(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(filename)) {
        soundBuffers[name] = buffer;
    }
}

void SoundManager::playSound(const std::string& name) {
    if (soundBuffers.find(name) == soundBuffers.end()) return;

    // Limit concurrent sounds for performance
    if (activeSounds.size() > 20) {
        activeSounds.pop_front();
    }

    activeSounds.emplace_back();
    sf::Sound& sound = activeSounds.back();

    sound.setBuffer(soundBuffers[name]);
    sound.setVolume(50.0f);
    sound.play();
}

void SoundManager::playMusic(const std::string& filename) {
    if (backgroundMusic.openFromFile(filename)) {
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(50.0f);
        backgroundMusic.play();
    }
}

void SoundManager::setMusicVolume(float volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    backgroundMusic.setVolume(volume);
}

float SoundManager::getMusicVolume() {
    return backgroundMusic.getVolume();
}

void SoundManager::update() {
    // Remove stopped sounds from the list
    activeSounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Stopped;
        });
}