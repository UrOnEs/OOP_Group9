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

void SoundManager::playMusic(const std::string& filename) {
    // Eðer zaten çalýyorsa ve ayný dosya ise durdurma
    // Ancak sf::Music dosya adýný tutmaz, o yüzden direkt açýyoruz.

    if (backgroundMusic.openFromFile(filename)) {
        backgroundMusic.setLoop(true); // Müzik bitince baþa dönsün
        backgroundMusic.setVolume(50.0f); // Varsayýlan ses
        backgroundMusic.play();
    }
    else {
        std::cerr << "Muzik acilamadi: " << filename << std::endl;
    }
}

void SoundManager::setMusicVolume(float volume) {
    // 0 ile 100 arasýnda sýnýrla
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    backgroundMusic.setVolume(volume);
}

float SoundManager::getMusicVolume() {
    return backgroundMusic.getVolume();
}

void SoundManager::update() {
    // Çalmasý biten sesleri listeden temizle (Memory leak olmamasý için)
    activeSounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Stopped;
        });
}