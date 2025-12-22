#include "Systems/SoundManager.h"
#include <iostream>

std::map<std::string, sf::SoundBuffer> SoundManager::soundBuffers;
std::list<sf::Sound> SoundManager::activeSounds;

void SoundManager::loadSound(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(filename)) {
        soundBuffers[name] = buffer;
    }
    else {
        std::cerr << "Ses yuklenemedi: " << filename << std::endl;
    }
}

void SoundManager::playSound(const std::string& name) {
    // Eðer ses buffer'da yoksa iþlem yapma
    if (soundBuffers.find(name) == soundBuffers.end()) return;

    // Ayný anda çok fazla ses varsa en eskisini sil (Performans korumasý)
    if (activeSounds.size() > 20) {
        activeSounds.pop_front();
    }

    // Listeye yeni bir ses ekle
    activeSounds.emplace_back();
    sf::Sound& sound = activeSounds.back();

    // Ayarlarý yap ve çal
    sound.setBuffer(soundBuffers[name]);
    sound.setVolume(50.0f); // Ýstersen parametre olarak alabilirsin
    // sound.setPitch(0.8f + (rand() % 40) / 100.0f); // Ýstersen rastgelelik katabilirsin
    sound.play();
}

void SoundManager::update() {
    // Çalmasý biten sesleri listeden temizle (Memory leak olmamasý için)
    activeSounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Stopped;
        });
}